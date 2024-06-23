#ifndef PE_TRADER_H
#define PE_TRADER_H

#include "pe_common.h"

#define ORDER_BUFLEN 20

typedef struct market_order {
    char order_type[ORDER_BUFLEN];
    char product[ORDER_BUFLEN];
    int qty;
    int price;
} mktorder_t;

/* 
 * Reads message from a file descriptor until it reads a <delim> character. 
 * If the buffer overflows (buflen - 1), it will read the pipe until it sees 
 * a delimiter, but will not update the buffer.
 */
void read_msg(int fd, char *buf, int buflen, char delim);
/* From a message string, parse MARKET SELL ORDER */
void parse_market_order(struct market_order *mo, char *msg);
/* Pause until response signal from <ppid> */
void pause_until_response(int ppid);
/* keep signalling <sig> until signal response from <ppid> */
void signal_until_response(int ppid, int sig);

#endif
