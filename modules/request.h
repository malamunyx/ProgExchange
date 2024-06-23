#ifndef REQUEST_H
#define REQUEST_H

#include "../pe_common.h"

/* Message Type */
typedef enum msg {
    MSG_BUY,
    MSG_SELL,
    MSG_AMEND,
    MSG_CANCEL,
    MSG_INVALID
} msg_t;

/* Request Struct */
typedef struct request {
    msg_t   type;
    int     trader_id;
    int     order_id;
    int     qty;
    int     price;
    char    product_name[PRODUCT_BUFLEN];
} request_t;



/* Helper Functions */

 /*
  * Helper function: count the amount of words in a message.
  */
int count_words(const char *str);
/* 
 * Helper function: returns a dynamically allocated string array 
 * by splitting the message into an array of words.
 */
char **get_str_vec(int wrd_cnt, const char *buf);
/*
 * Helper function: parses a string into an integer.
 */
int get_number(const char *str);


/* 
 * Request String Parsing: validates the message,
 * ensuring the message format is correct, and 
 * parameters are within bounds.
 */
request_t request_parse(const char *str, int trader_id);

#endif