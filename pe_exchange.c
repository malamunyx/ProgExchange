/**
 * comp2017 - assignment 3
 * Mikael Sebastian Indrawan
 * mind7537
 */

#include "pe_exchange.h"
#include "modules/orderbook.h"
#include "pe_common.h"
#include <stdint.h>

/* pid ring buffer */
rbuf_t PID_QUEUE;

void sigusr_handler(int signo, siginfo_t *siginfo, void *context)
{
    // Enqueue SI_PID
    rbuf_append(&PID_QUEUE, siginfo->si_pid);
}

void sigpipe_handler(int signo, siginfo_t *siginfo, void *context)
{
    // IGNORE SIGPIPE SIGNAL
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, LOG_ERROR" Insufficient argv parameter count\n");
        exit(EXIT_FAILURE);
    }

    /* Exchange variables */
    int epfd = epoll_create1(0);
    struct epoll_event events[MAX_EVENTS];
    orderbook_t pex = {
        .fees 			= 0,
        .n_products 	= 0,
        .n_traders 		= (argc - 2),
        .product_list 	= NULL,
        .trader_list 	= NULL
    };
    struct sigaction sa = {
        .sa_flags 		= SA_SIGINFO | SA_RESTART,
        .sa_sigaction 	= sigusr_handler
    };
    struct sigaction sp_sa = {
        .sa_flags 		= SA_SIGINFO | SA_RESTART,
        .sa_sigaction 	= sigpipe_handler
    };
    rbuf_init(&PID_QUEUE);

    /* Init Sigaction */
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGPIPE, &sp_sa, NULL);

    /* Startup */
    printf(LOG_PREFIX" Starting\n");

    /* Memory allocate and loads product list */
    load_exchange_products(&pex, argv[1]);
    if (pex.n_products == 0 || pex.product_list == NULL) {
        fprintf(stderr, LOG_ERROR" Invalid product file\n");
        exit(EXIT_FAILURE);
    }
    print_exchange_products(&pex);

    /* Memory allocate trader_list */
    pex.trader_list = malloc((pex.n_traders) * sizeof(trader_t));
    if (pex.trader_list == NULL) {
        fprintf(stderr, LOG_ERROR" Cannot memory allocate trader list\n");
        free(pex.product_list); 
        exit(EXIT_FAILURE);
    }
    /* Execute child processes (Traders) */
    if (exec_traders(&pex, epfd, argv) < 0) {
        fprintf(stderr, LOG_ERROR" Error in trader exec setup\n");
        free(pex.product_list);
        free(pex.trader_list);
        exit(EXIT_FAILURE);
    }

    usleep(1000000);

    /* Notify [MARKET OPEN] */
    for (int i = 0; i < pex.n_traders; i++) {
        trader_t *t = &pex.trader_list[i];
        char *msg = "MARKET OPEN;";
        write(t->exchange_fd, msg, strlen(msg));
        kill(t->pid, SIGUSR1);
    }

    /* Event Loop */
    char buf[MSG_BUFLEN] = {0};
    int connections = pex.n_traders;
    
    while (connections > 0) {
        int ev_cnt = epoll_wait(epfd, events, MAX_EVENTS, 500);
        
        for (int i = 0; i < ev_cnt; i++) {
            
            trader_t *t = events[i].data.ptr;

            if (events[i].events & EPOLLHUP) {

                disconnect_trader(t, epfd, events);
                --connections;

            } else if ((t->pid == rbuf_peek(&PID_QUEUE)) && (events[i].events & EPOLLIN)) {
                
                rbuf_remove(&PID_QUEUE);
                read_msg(t->trader_fd, buf, MSG_BUFLEN, ';');
                buf[strcspn(buf, ";")] = 0;

                printf(LOG_PREFIX" [T%d] Parsing command: <%s>\n", t->id, buf);
                request_t req = request_parse(buf, t->id);
                exec_order(&pex, &req);
            }
        }
    }

    printf(LOG_PREFIX" Trading completed\n");
    printf(LOG_PREFIX" Exchange fees collected: $%ld\n", pex.fees);
    
    orderbook_free(&pex);

    return 0;
}

void load_exchange_products(orderbook_t *ord, char *pfname)
{
    char *sptr, buf[PRODUCT_BUFLEN];
    int prd_cnt;

    FILE *pf = fopen(pfname, "r");

    if (pf == NULL) {
        return;
    }

    fgets(buf, PRODUCT_BUFLEN, pf);

    prd_cnt = strtol(buf, &sptr, 10);

    if (prd_cnt < 0) {
        fprintf(stderr, LOG_ERROR" Invalid product quantity\n");
        return;
    }
    
    /* Initialise exchange product list variables */
    ord->n_products = prd_cnt;
    ord->product_list = malloc(prd_cnt * sizeof(product_t));

    if (ord->product_list == NULL) {
        fprintf(stderr, LOG_ERROR" Failed malloc\n");
        return;
    }

    int i = 0;
    while (i < prd_cnt && fgets(buf, PRODUCT_BUFLEN, pf) != NULL) {
        buf[strcspn(buf, "\r\n")] = 0;

        if (strlen(buf) > 16)
            break;
        
        product_init(&ord->product_list[i], buf, i);

        ++i;
    }

    if (i < prd_cnt) {
        fprintf(stderr, LOG_ERROR" Product quantity mismatch\n");
        free(ord->product_list);
        ord->product_list = NULL;
        ord->n_products = -1;
        return;
    }

    fclose(pf);
}

void print_exchange_products(orderbook_t *ord)
{
    printf(LOG_PREFIX" Trading %d products: ", ord->n_products);
    for (int i = 0; i < ord->n_products; i++) {
        printf("%s%s", 
            ord->product_list[i].name,
            (i == ord->n_products - 1) ? "\n" : " "
        );
    }
}

void read_msg(int fd, char *buf, int buflen, char delim)
{
    if (buf == NULL) {
        return;
    }

    int idx = 0;
    char c;
    do {
        read(fd, &c, 1);

        if (idx < buflen - 1)
            buf[idx++] = c;

    } while (c != delim);

    buf[idx] = '\0';
}

int exec_traders(orderbook_t *ord, 	int epfd, char **argv)
{
    for (int i = 0; i < ord->n_traders; i++) {

        trader_t *tr = &ord->trader_list[i];

        struct epoll_event es = {0};
        es.events = EPOLLIN | EPOLLHUP; // Not necessary to add EPOLLHUP into events
        es.data.ptr = tr;

        trader_init(tr, i, ord->n_products);

        unlink(tr->exchange_fifo);
        unlink(tr->trader_fifo);

        if (mkfifo(tr->exchange_fifo, 0666) < 0) {
            fprintf(stderr, LOG_ERROR);
            perror(" mkfifo");
            return -1;
        }
        printf(LOG_PREFIX" Created FIFO %s\n", tr->exchange_fifo);
        if (mkfifo(tr->trader_fifo, 0666) < 0) {
            fprintf(stderr, LOG_ERROR);
            perror(" mkfifo");
            return -1;
        }
        printf(LOG_PREFIX" Created FIFO %s\n", tr->trader_fifo);

        int pid = fork();
        if (pid < 0) { // ERROR

            fprintf(stderr, LOG_ERROR);
            perror(" fork");
            return -1;

        } else if (pid == 0) { // CHILD

            char idstr[11]; // Max 10 bytes + '\0'
            snprintf(idstr, 11, "%d", i);

            printf(LOG_PREFIX" Starting trader %d (%s)\n", i, argv[i + 2]);
            execl(argv[i + 2], argv[i + 2], idstr, NULL);

            fprintf(stderr, LOG_ERROR);
            perror(" execl");

            for (int i = 0; i < ord->n_traders; i++) {
                unlink(ord->trader_list[i].exchange_fifo);
                unlink(ord->trader_list[i].trader_fifo);
            }

            killpg(0, SIGTERM); // Kill all in process group
            return -1;

        } else {
            // Track the process ID of trader
            tr->pid = pid;

            tr->exchange_fd = open(tr->exchange_fifo, O_WRONLY);
            if (tr->exchange_fd < 0) {
                perror("open (exchange_fd)");
                return -1;
            }
            printf(LOG_PREFIX" Connected to %s\n", tr->exchange_fifo);

            tr->trader_fd = open(tr->trader_fifo, O_RDONLY);
            if (tr->trader_fd < 0) {
                perror("open (trader_fd)");
                return -1;
            }
            printf(LOG_PREFIX" Connected to %s\n", tr->trader_fifo);
            
            int eplret = epoll_ctl(epfd, EPOLL_CTL_ADD, tr->trader_fd, &es);

            if (eplret < 0) {
                fprintf(stderr, LOG_ERROR);
                perror(" epoll_ctl");
                return -1;
            }

        }
    }
    return 0;
}

void disconnect_trader(trader_t *t, int epfd, struct epoll_event *events)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, t->trader_fd, events);
    printf(LOG_PREFIX" Trader %d disconnected\n", t->id);
    close(t->trader_fd);
    t->pid = INT32_MIN;
}