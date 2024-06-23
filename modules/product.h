#ifndef PRODUCT_H
#define PRODUCT_H

#include "../pe_common.h"

/* 
 * Type of order <BUY/SELL> that 
 * the node represents.
 */
typedef enum nodetype {
    BUY_NODE,
    SELL_NODE
} nodetype_t;

/* 
 * Product Order List: the pqueue struct represents a 
 * ring, doubly linked list priority queue. From head to 
 * tail, they are ranked by price.
 */

 /* Priority Queue Structure */
typedef struct node {                       //
    nodetype_t      type;                   //
    int             trader_id;              //
    int             order_id;               //
    int             qty;                    //
    int             price;                  //
    struct node     *next;                  //
    struct node     *prev;                  //
} node_t;                                   //
typedef struct pqueue {                     //
    node_t      sentinel;                   //
} pqueue_t;                                 //

/* Product Structure */                     //
typedef struct product {                    //
    int         product_id;                 //
    char        name[PRODUCT_BUFLEN];       //
    pqueue_t    orders;                     //
} product_t;                                //

/* Priority Queue Functions */

/* 
 * Initialises a stack-allocated priority queue struct.
 * Sets attributes to default values and an initialises 
 * the sentinel node of the orders list.
 */
void pq_init(pqueue_t *pq);
/*
 * Initialises the order node for the product order list. 
 */
node_t *init_node(nodetype_t type, int trader_id, int order_id, int qty, int price);
/* 
 * Traverses through the orders list, returning a 
 * node with <trader_id> and <order_id> attributes.
 */
node_t *pq_get(pqueue_t *pq, int trader_id, int order_id);
/* 
 * Enqueue a BUY_NODE order from the rear, ensuring it
 * maintains the ordering of PEX price matching priority.
 */
void pq_enqueue_buy(pqueue_t *pq, node_t *n);
/* 
 * Enqueue a SELL_NODE order from the front, ensuring it
 * maintains the ordering of PEX price matching priority.
 */
void pq_enqueue_sell(pqueue_t *pq, node_t *n);
/* 
 * Removes a node from the pqueue.
 * <del> MUST be freed after removal.
 */
void pq_remove(pqueue_t *pq, node_t *del);
/*
 * Frees any present nodes, except for the sentinel node
 */
void pq_free(pqueue_t *pq);



/* Product Functions */

/* 
 * Traverses through the orders list of <pr>, returning a 
 * node with <trader_id> and <order_id> attributes.
 * This is a wrapper function for function pq_get;
 */
node_t *get_order(product_t *pr, int trader_id, int order_id);
/* 
 * Initialises the product struct. Sets attributes to default  
 * values and an initialises the orders list.
 */
void product_init(product_t *pr, const char *name, int product_id);



/* Product Utilities */

/* 
 * Frees the product node.
 */
void product_free(product_t *pr);

#endif