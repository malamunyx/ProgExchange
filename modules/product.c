#include "product.h"

void pq_init(pqueue_t *pq) 
{
    pq->sentinel.next = &pq->sentinel;
    pq->sentinel.prev = &pq->sentinel;
    pq->sentinel.trader_id = INT32_MIN;
    pq->sentinel.order_id = INT32_MIN;
    pq->sentinel.qty = INT32_MIN;
    pq->sentinel.price = INT32_MIN;
}

node_t *init_node(nodetype_t type, int trader_id, int order_id, int qty, int price) 
{
    struct node *tmp = malloc(sizeof(struct node));

    tmp->type = type;
    tmp->trader_id = trader_id;
    tmp->order_id = order_id;
    tmp->qty = qty;
    tmp->price = price;
    tmp->prev = NULL;
    tmp->next = NULL;

    return tmp;
}

node_t *pq_get(pqueue_t *pq, int trader_id, int order_id) 
{
    node_t *cursor = pq->sentinel.next;

    while (cursor != &pq->sentinel) {
        if (trader_id == cursor->trader_id 
                && order_id == cursor->order_id) {
            return cursor;
        }
        cursor = cursor->next;
    }
    // Back to Sentinel
    return NULL;
}

void pq_enqueue_buy(pqueue_t *pq, node_t *n) 
{
    node_t *cursor = pq->sentinel.prev; // From the rear (min)
    
    while (cursor != &pq->sentinel) {
        if (n->price <= cursor->price) {
            break;
        }
        
        cursor = cursor->prev;
    }

    n->next = cursor->next;
    n->prev = cursor;

    n->next->prev = n;
    n->prev->next = n;
}

void pq_enqueue_sell(pqueue_t *pq, node_t *n) 
{
    node_t *cursor = pq->sentinel.next;
    
    while (cursor != &pq->sentinel) {
        if (n->price >= cursor->price) {
            break;
        }
        
        cursor = cursor->next;
    }

    n->next = cursor;
    n->prev = cursor->prev;

    n->next->prev = n;
    n->prev->next = n;
}

void pq_remove(pqueue_t *pq, node_t *del)
{
    if (del == &pq->sentinel)
        return;
    
    del->prev->next = del->next;
    del->next->prev = del->prev;

    // free(del);
}

void pq_free(pqueue_t *pq)
{
    node_t *del = NULL;

    while (pq->sentinel.next != &pq->sentinel) {
        del = pq->sentinel.next;
        pq_remove(pq, del);
        free(del);
    }
}



void product_init(product_t *pr, const char *name, int product_id)
{
    pr->product_id = product_id;
    strncpy(pr->name, name, PRODUCT_BUFLEN);
    pq_init(&pr->orders);
}

void product_free(product_t *pr)
{
    pq_free(&pr->orders);
}
