#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "product.h"
#include "request.h"
#include "trader.h"

#define LOG_PREFIX "[PEX]"
#define LOG_ERROR "[ERR]"



/* 
 * Orderbook Struct: contains relevant records and 
 * information of PEX traders, orders, transactions
 * and fees.
 */
typedef struct orderbook {
    long int    fees;
    int         n_traders;
    int         n_products;
    trader_t    *trader_list;
    product_t   *product_list;
} orderbook_t;



/* Getters */

/* 
 * Orderbook Entity Getter: from string <prd_name>, returns 
 * a pointer to a product in the orderbook.
 */
product_t *str_get_product(orderbook_t *ord, const char *prd_name);
/* 
 * Orderbook Entity Getter: from the <trader_id> and <order_id> integers,
 * returns a pointer to a product_t struct containing the order with <trader_id> 
 * and <order_id> in the orderbook.
 */
product_t *id_get_product(orderbook_t *ord, int trader_id, int order_id);
/* 
 * Orderbook Entity Getter: from the <trader_id> integer,
 * returns a pointer to a product_t struct in the orderbook.
 */
trader_t *get_trader(orderbook_t *ord, int trader_id);
/* 
 * Orderbook Entity Getter: from the <trader_id> integer,
 * returns a pointer to a product_t struct in the orderbook.
 */
node_t *get_order(product_t *pr, int trader_id, int order_id);



/* Transactions */

/* 
 * Orderbook Transaction: adjusts respective traders' positions, and
 * subsequently prints a price matching report to stdout.
 * This only occurs when buyer is the last person to place a "matched" order.
 */
void transact_buy(orderbook_t *ord, product_t *pr, trader_t *buyer, 
                    trader_t *seller, node_t *order, int buy_id, int ord_qty);
/* 
 * Orderbook Transaction: adjusts respective traders' positions, and
 * subsequently prints a price matching report to stdout.
 * This only occurs when seller is the last person to place a "matched" order.
 */
void transact_sell(orderbook_t *ord, product_t *pr, trader_t *buyer, 
                    trader_t *seller, node_t *order, int sell_id, int ord_qty);



/* Execution */

/* 
 * Order Execution: Validates order request <req> relative 
 * to orderbook <ord>, then executes the respective order.
 */
void exec_order(orderbook_t *ord, request_t *req);
    /* 
     * Buy Order Execution: Traverses order list from the lowest price to price match. 
     * If a price match exists, a transaction occurs. Remaining orders after price matching
     * sequence with quantites > 0 get inserted into the product order list from the rear
     * (lowest price).
     */
    void order_buy(orderbook_t *ord, product_t *pr, request_t *req);
    /* 
     * Sell Order Execution: Traverses order list from the highest price to price match. 
     * If a price match exists, a transaction occurs. Remaining orders after price matching
     * sequence with quantites > 0 get inserted into the product order list from the front
     * (highest price).
     */
    void order_sell(orderbook_t *ord, product_t *pr, request_t *req);
    /* 
     * Amend Order Execution: removes order node from the order list. Depending on the
     * type of the original order node, a <BUY/SELL> order is executed with the amended
     * order information.
     */
    void order_amend(orderbook_t *ord, product_t *pr, request_t *req);
    /* 
     * Cancel Order Execution: removes order node from the order list. 
     */
    void order_cancel(orderbook_t *ord, product_t *pr, request_t *req);



/* Notifications (Message Traders) */

/* 
 * Trader Notification: Send accept message to <tr>,
 * and sends market order to other traders. 
 * This notification is only for <BUY/SELL> orders.
 */
void notify_accepted(orderbook_t *ord, product_t *pr, request_t *req, trader_t *tr);
/* 
 * Trader Notification: Send amended message to <tr>,
 * and sends market order to other traders.
 * This notification is only for <AMEND> orders.
 */
void notify_amended(orderbook_t *ord, product_t *pr, request_t *req, trader_t *tr);
/*
 * Trader Notification: Send cancelled message to <tr>,
 * and sends market order to other traders.
 * This notification is only for <CANCEL> orders.
 */
void notify_cancelled(orderbook_t *ord, product_t *pr, request_t *req, trader_t *tr);
/*
 * Trader Notification: Send cancelled message to <tr>.
 * This notification is only for <CANCEL> orders.
 */
void notify_invalid(orderbook_t *ord, trader_t *tr);
/*
 * Trader Notification: Send fill message to respective <buyer> and 
 * <seller> traders of the transaction.
 * This notification only occurs during order matching procedure.
 */
void notify_fill(trader_t *buyer, trader_t *seller, int buy_order_id, int sell_order_id, int qty);


/* Orderbook Reporting */

/* 
 * Orderbook Report: Gets the amount of distinct prices in <BUY/SELL> orders.
 */
int get_level(product_t *pr, nodetype_t type);
/* 
 * Orderbook Report: prints each order level and its quantities.
 */
void print_product_orders(product_t *pr);
/* 
 * Orderbook Report: Prints the whole orderbook report to stdout.
 */
void print_report(orderbook_t *ord);

/* Utilities */

/*
 * Orderbook Utility: frees allocated memory in orderbook.
 */
void orderbook_free(orderbook_t *ord);

#endif