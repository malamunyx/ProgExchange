#ifndef TRADER_H
#define TRADER_H

#include "../pe_common.h"

typedef struct trader {
    pid_t    	pid;
    int     	id;
    int     	order_id;
    int     	exchange_fd;
    int     	trader_fd;
    int			product_qty;
    long int	*positions;
    long int	*positions_qty;
    char    exchange_fifo[FIFO_NAMELEN];
    char    trader_fifo[FIFO_NAMELEN];
} trader_t;


/*
 * Initialises trader struct pointed by <t>, setting default
 * attributes. Also dynamically allocates the positions and 
 * positions quantity list.
 * Must be freed prior to program termination.
 */
void trader_init(trader_t *t, int id, int product_qty);

#endif