#include "trader.h"
#include <stdlib.h>

void trader_init(trader_t *t, int id, int product_qty) {
    t->id = id;
    t->order_id = 0;
    t->pid = INT32_MIN;
    
    t->exchange_fd = INT32_MIN;
    t->trader_fd = INT32_MIN;

    t->product_qty = product_qty;
    t->positions = calloc(product_qty, sizeof(long int));
    t->positions_qty = calloc(product_qty, sizeof(long int));
    
    snprintf(t->exchange_fifo, FIFO_NAMELEN, FIFO_EXCHANGE, id);
    snprintf(t->trader_fifo, FIFO_NAMELEN, FIFO_TRADER, id);
}