#include "../../pe_trader.h"
#include <unistd.h>

void handler(int signo, siginfo_t *siginfo, void *context) {
}

void my_read(int fd, char *buf, int buflen, char delim)
{
	int idx = 0;
	char c;
	do {
		read(fd, &c, 1);

		if (idx < buflen - 1)
			buf[idx++] = c;

	} while (c != delim);

	buf[idx] = '\0';
}


int main(int argc, char ** argv) {
	
	if (argc < 2) {
		return 1;
	}

	char *sptr;
	int tr_id = strtol(argv[1], &sptr, 10);

	if (sptr == argv[1]) {
		return 1;
	}
	if (tr_id < 0) {
		return 1;
	}

	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;

	sigaction(SIGUSR1, &sa, NULL);

	char ptc_fifo[FIFO_NAMELEN]; 
	char ctp_fifo[FIFO_NAMELEN]; 
	int ptc_fd = -1, ctp_fd = -1;

	snprintf(ptc_fifo, FIFO_NAMELEN, FIFO_EXCHANGE, tr_id);
	snprintf(ctp_fifo, FIFO_NAMELEN, FIFO_TRADER, tr_id);

	/* trader initialise complete, establish connection */
	if ((ptc_fd = open(ptc_fifo, O_RDONLY)) == -1) {
		perror("open <ptc_fifo>");
		return 1;
	}

	if ((ctp_fd = open(ctp_fifo, O_WRONLY)) == -1) {
		perror("open <ctp_fifo>");
		return 1;
	}

	char buf[MSG_BUFLEN];

	// OPEN
	pause();
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	// SEND
	char *msg = "SELL 0 GPU 500 120;";
	write(ctp_fd, msg, strlen(msg));
	kill(getppid(), SIGUSR1);


	// ACCEPTED 
	pause();
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	pause();
	//MARKET
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	//FILL
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	sleep(1);

	msg = "SELL 1 Router 700 200;";
	write(ctp_fd, msg, strlen(msg));
	kill(getppid(), SIGUSR1);

	//ACCEPTED
	pause();
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	pause();
	//MARKET
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	//FILL
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	sleep(1);

	msg = "SELL 2 GPU 1000 250;";
	write(ctp_fd, msg, strlen(msg));
	kill(getppid(), SIGUSR1);

	pause();
	my_read(ptc_fd, buf, MSG_BUFLEN, ';');

	close(ctp_fd);
	close(ptc_fd);

	return 0;
}