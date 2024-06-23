#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"
#include "modules/orderbook.h"
#include "modules/rbuf.h"

/* 
 * Reads textfile <pfname>, dynamically allocating the orderbook 
 * product list
 */
void load_exchange_products(orderbook_t *ord, char *pfname);
/* Prints the quantity and name of products that in the orderbook */
void print_exchange_products(orderbook_t *ord);
/* 
 * Reads message from a file descriptor until it reads a <delim> character. 
 * If the buffer overflows (buflen - 1), it will read the pipe until it sees 
 * a delimiter, but will not update the buffer.
 */
void read_msg(int fd, char *buf, int buflen, char delim);
/* 
 * Creates and connect FIFOs, forking and executing trader binaries and 
 * establish epoll events for each trader FIFO file descriptor
 * (Trader -> Exchange). Epoll data pointer is referenced to the relevant
 * trader struct.
 */
int exec_traders(orderbook_t *ord, 	int epfd, char **argv);
/* 
 * Upon trader disconnect (trader-end FIFO close), close and remove trader file  
 * descriptor from epoll interest list. Also resets the trader PID to -1.
 */
void disconnect_trader(trader_t *t, int epfd, struct epoll_event *events);

#endif
