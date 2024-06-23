#include "pe_trader.h"
#include "pe_common.h"
#include "pe_exchange.h"
#include <signal.h>
#include <unistd.h>

static int SIG_PID = DEFAULT_SIG_PID;

void sigusr_handler(int signo, siginfo_t *siginfo, void *context)
{
    SIG_PID = siginfo->si_pid;
}

int main(int argc, char ** argv)
{
    /* Trader variables */
    char ptc_fifo[FIFO_NAMELEN]; 
    char ctp_fifo[FIFO_NAMELEN]; 
    int ptc_fd = -1, ctp_fd = -1;
    int trader_id, order_id = 0;  

    if (argc < 2) {
        return 1;
    }

    char *sptr;
    trader_id = strtol(argv[1], &sptr, 10);

    if (sptr == argv[1]) {
        return 1;
    } 
    if (trader_id < 0) {
        return 1;
    }

    snprintf(ptc_fifo, FIFO_NAMELEN, FIFO_EXCHANGE, trader_id);
    snprintf(ctp_fifo, FIFO_NAMELEN, FIFO_TRADER, trader_id);

    /* Init signal handler */
    struct sigaction sa = {
        .sa_flags = SA_SIGINFO | SA_RESTART,
        .sa_sigaction = sigusr_handler
    };
    sigaction(SIGUSR1, &sa, NULL);

    /* Trader initialise complete, establish connection */
    if ((ptc_fd = open(ptc_fifo, O_RDONLY)) == -1) {
        perror("open <ptc_fifo>");
        return 1;
    }

    if ((ctp_fd = open(ctp_fifo, O_WRONLY)) == -1) {
        perror("open <ctp_fifo>");
        return 1;
    }

    /* Record recent market order */
    mktorder_t msg = {0};
    char buf_in[MSG_BUFLEN] = {0};
    char buf_out[MSG_BUFLEN] = {0};

    /* Market OPEN msg */
    pause_until_response(getppid());
    read(ptc_fd, buf_out, MSG_BUFLEN);

    /* Trader Event Loop */
    int idx;
    while (1) {
        do {
            pause_until_response(getppid());
            // idx = read(ptc_fd, buf_in, MSG_BUFLEN);
            // buf_in[idx] = 0;
            read_msg(ptc_fd, buf_in, MSG_BUFLEN, ';');
        } while (strncmp(buf_in, "MARKET SELL ", 12) != 0);

        /* Parse Order */
        parse_market_order(&msg, buf_in);

        if (msg.qty >= 1000)
            break;
        
        /* Send (BUY) order */
        snprintf(buf_out, MSG_BUFLEN, "BUY %d %s %d %d;",
           order_id, msg.product, msg.qty, msg.price);

        write(ctp_fd, buf_out, strlen(buf_out));
        signal_until_response(getppid(), SIGUSR1);

        /* Read ACCEPTED Message */
        idx = read(ptc_fd, buf_in, MSG_BUFLEN);
        buf_in[idx] = 0;
        
        /* Fill is handled by do_while loop checking for market sell. */
        ++order_id;
    }
    

    close(ptc_fd);
    close(ctp_fd);

    return 0;
}

/* 
 * Reads message from a file descriptor until it reads a <delim> character. 
 * If the buffer overflows (buflen - 1), it will read the pipe until it sees 
 * a delimiter, but will not update the buffer.
 */
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

void parse_market_order(struct market_order *mo, char *msg)
{
    char *start, *parse, *tok, *ptr;

    char *str = msg + strlen("MARKET "); // Offset msg.
    start = parse = strdup(str);
    
    tok = strsep(&parse, " ");
    strcpy(mo->order_type, tok);

    tok = strsep(&parse, " ");
    strcpy(mo->product, tok);

    tok = strsep(&parse, " ");
    mo->qty = strtol(tok, &ptr, 10);

    tok = strsep(&parse, " ");
    tok[strcspn(tok, ";")] = 0;
    mo->price = strtol(tok, &ptr, 10);

    free(start);
}

void pause_until_response(int ppid)
{
    do {
        pause();
    } while (SIG_PID != ppid);
    // SIG_PID BACK TO DEFAULT
    SIG_PID = DEFAULT_SIG_PID;
}


void signal_until_response(int ppid, int sig)
{
    do {
        kill(ppid, sig);
        usleep(250000);	
    } while (SIG_PID != ppid);
    // SIG_PID BACK TO DEFAULT
    SIG_PID = DEFAULT_SIG_PID;
}
