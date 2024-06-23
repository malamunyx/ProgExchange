/*
Mikael Sebastian Indrawan 
520484597
mind7537
USYD CODE CITATION ACKNOWLEDGEMENT
This file contains acknowledgements of code
*/

#include "orderbook.h"
#include "trader.h"

// USYD CODE CITATION ACKNOWLEDGEMENT
// I declare that the majority of the following code has been taken
// from the website titled: "Rounding integer division (instead of truncating)" 
// and it is not my own work. 
//  
// Original URL
// https://stackoverflow.com/questions/2422712/rounding-integer-division-instead-of-truncating
// Last access May, 2023
// Helper function: rounded_int_division(long int dividend, long int divisor);

long int rounded_int_division(long int dividend, long int divisor)
{
    return (dividend + (divisor / 2)) / divisor;
}

long int get_total_cost(int quantity, int price)
{
    return (long int) quantity * (long int) price;
}

long int get_transaction_fee(long int total)
{
    return rounded_int_division(total, 100) * FEE_PERCENTAGE;
}



product_t *str_get_product(orderbook_t *ord, const char * prd_name)
{
    if (ord == NULL) {
        return NULL;
    }

    for (int i = 0; i < ord->n_products; i++) {
        if (strcmp(prd_name, ord->product_list[i].name) == 0) {
            return &ord->product_list[i];
        }
    }

    return NULL;
}

product_t *id_get_product(orderbook_t *ord, int trader_id, int order_id)
{
    if (ord == NULL) {
        return NULL;
    }

    for (int i = 0; i < ord->n_products; i++) {
        if (get_order(&ord->product_list[i], trader_id, order_id) != NULL) {
            return &ord->product_list[i];
        }
    }

    return NULL;
}

trader_t *get_trader(orderbook_t *ord, int trader_id)
{
    if (ord == NULL || trader_id < 0 || trader_id > ord->n_traders) {
        return NULL;
    }

    return &(ord->trader_list[trader_id]);
}

node_t *get_order(product_t *pr, int trader_id, int order_id)
{
    if (pr == NULL) {
        return NULL;
    }

    return pq_get(&pr->orders, trader_id, order_id);
}



void transact_buy(orderbook_t *ord, product_t *pr, trader_t *buyer, 
                    trader_t *seller, node_t *order, int buy_id, int ord_qty)
{
    if (ord == NULL || pr == NULL || buyer == NULL || seller == NULL || order == NULL) {
        return;
    }

    long int total = get_total_cost(ord_qty, order->price);
    long int fees = get_transaction_fee(total);

    printf(LOG_PREFIX" Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n",
        order->order_id,
        order->trader_id,
        buy_id,
        buyer->id,
        total,
        fees
    );

    buyer->positions_qty[pr->product_id] += ord_qty;
    buyer->positions[pr->product_id] -= (total + fees);

    seller->positions_qty[pr->product_id] -= ord_qty;
    seller->positions[pr->product_id] += total;

    ord->fees += fees;
}

void transact_sell(orderbook_t *ord, product_t *pr, trader_t *buyer, 
                    trader_t *seller, node_t *order, int sell_id, int ord_qty)
{
    if (ord == NULL || pr == NULL || buyer == NULL || seller == NULL || order == NULL) {
        return;
    }

    long int total = get_total_cost(ord_qty, order->price);
    long int fees = get_transaction_fee(total);

    printf(LOG_PREFIX" Match: Order %d [T%d], New Order %d [T%d], value: $%ld, fee: $%ld.\n",
        order->order_id,
        order->trader_id,
        sell_id,
        seller->id,
        total,
        fees
    );

    buyer->positions_qty[pr->product_id] += ord_qty;
    buyer->positions[pr->product_id] -= total;

    seller->positions_qty[pr->product_id] -= ord_qty;
    seller->positions[pr->product_id] += (total - fees);

    ord->fees += fees;
}



void exec_order(orderbook_t *ord, request_t *req)
{
    if (ord == NULL || req == NULL) {
        return;
    }

    /* Orderbook validation: Check product exists and order_id is valid */
    product_t *pr = NULL;
    trader_t *t = get_trader(ord, req->trader_id);

    switch (req->type) {
        case MSG_BUY:
        case MSG_SELL:
            pr = (t->order_id == req->order_id) ?
                str_get_product(ord, req->product_name) :
                NULL;
            break;
        case MSG_AMEND:
        case MSG_CANCEL:
            pr = id_get_product(ord, req->trader_id, req->order_id);
            break;
        default: // INVALID
            break;
    }
    
    if (pr == NULL)
        req->type = MSG_INVALID;

    /* Valid types */
    switch (req->type) {
        case MSG_BUY:
            notify_accepted(ord, pr, req, t);
            order_buy(ord, pr, req);
            t->order_id += 1;
            print_report(ord);
            return;
        case MSG_SELL:
            notify_accepted(ord, pr, req, t);
            order_sell(ord, pr, req);
            t->order_id += 1;
            print_report(ord);
            return;
        case MSG_AMEND:
            notify_amended(ord, pr, req, t);
            order_amend(ord, pr, req);
            print_report(ord);
            return;
        case MSG_CANCEL:
            notify_cancelled(ord, pr, req, t);
            order_cancel(ord, pr, req);
            print_report(ord);
            return;
        default: // INVALID
            notify_invalid(ord, t);
            break;
    }
}

void order_buy(orderbook_t *ord, product_t *pr, request_t *req)
{
    if (ord == NULL || pr == NULL || req == NULL) {
        return;
    }

    trader_t *buyer = get_trader(ord, req->trader_id);
    node_t *sentinel = &pr->orders.sentinel;
    int buy_qty = req->qty;
    int buy_price = req->price;
    node_t *cursor = sentinel->prev;
    
    while (buy_qty > 0 && cursor != sentinel && buy_price >= cursor->price) {

        trader_t *seller = get_trader(ord, cursor->trader_id);

        if (cursor->type == BUY_NODE) {
            cursor = cursor->prev; 
            continue;
        } 

        if (cursor->qty > buy_qty) {
            cursor->qty -= buy_qty;
            transact_buy(ord, pr, buyer, seller, cursor, req->order_id, buy_qty);
            notify_fill(buyer, seller, req->order_id, cursor->order_id, buy_qty);
            buy_qty = 0;

        } else {
            buy_qty -= cursor->qty;
            transact_buy(ord, pr, buyer, seller, cursor, req->order_id, cursor->qty);
            notify_fill(buyer, seller, req->order_id, cursor->order_id, cursor->qty);

            node_t *del = cursor;
            cursor = cursor->prev;
            pq_remove(&pr->orders, del);
            free(del);
        }
    }

    if (buy_qty > 0) {
        node_t *tmp = init_node(BUY_NODE, req->trader_id, req->order_id,
                            buy_qty, buy_price);
        pq_enqueue_buy(&pr->orders, tmp);
    }
}

void order_sell(orderbook_t *ord, product_t *pr, request_t *req)
{
    if (ord == NULL || pr == NULL || req == NULL) {
        return;
    }

    trader_t *seller = get_trader(ord, req->trader_id);
    node_t *sentinel = &pr->orders.sentinel;
    int sell_qty = req->qty;
    int sell_price = req->price;
    node_t *cursor = sentinel->next;
    
    while (sell_qty > 0 && cursor != sentinel && sell_price <= cursor->price) {

        trader_t *buyer = get_trader(ord, cursor->trader_id);
        
        if (cursor->type == SELL_NODE) {
            cursor = cursor->next;
            continue;
        } 

        if (cursor->qty > sell_qty) { // END LOOP
            cursor->qty -= sell_qty;
            transact_sell(ord, pr, buyer, seller, cursor, req->order_id, sell_qty);
            notify_fill(buyer, seller, cursor->order_id, req->order_id, sell_qty);
            sell_qty = 0;

        } else {
            sell_qty -= cursor->qty;
            transact_sell(ord, pr, buyer, seller, cursor, req->order_id, cursor->qty);
            notify_fill(buyer, seller, cursor->order_id, req->order_id, cursor->qty);
            
            node_t *del = cursor;
            cursor = cursor->next;
            pq_remove(&pr->orders, del);
            free(del);
        }
    }

    if (sell_qty > 0) {
        node_t *tmp = init_node(SELL_NODE, req->trader_id, req->order_id, 
                            sell_qty, sell_price);
        pq_enqueue_sell(&pr->orders, tmp);
    }
}

void order_amend(orderbook_t *ord, product_t *pr, request_t *req)
{
    if (ord == NULL || pr == NULL || req == NULL) {
        return;
    }

    /* Get node pointer, remove node, buy/sell node with req info */
    node_t *tmp = get_order(pr, req->trader_id, req->order_id);
    pq_remove(&pr->orders, tmp);

    if (tmp->type == BUY_NODE) {
        order_buy(ord, pr, req);
    } else {
        order_sell(ord, pr, req);
    }

    free(tmp);
}

void order_cancel(orderbook_t *ord, product_t *pr, request_t *req)
{
    if (ord == NULL || pr == NULL || req == NULL) {
        return;
    }

    node_t *tmp = get_order(pr, req->trader_id, req->order_id);
    pq_remove(&pr->orders, tmp);
    free(tmp);
}



void notify_accepted(orderbook_t *ord, product_t *pr, request_t *req, trader_t *tr)
{
    if (ord == NULL || pr == NULL || req == NULL || tr == NULL) {
        return;
    }

    char msg[MSG_BUFLEN];
    snprintf(msg, MSG_BUFLEN, "ACCEPTED %d;", req->order_id);
    
    write(tr->exchange_fd, msg, strlen(msg));
    kill(tr->pid, SIGUSR1);

    snprintf(msg, MSG_BUFLEN, "MARKET %s %s %d %d;", 
        (req->type == MSG_BUY) ? "BUY" : "SELL",
        req->product_name,
        req->qty,
        req->price
    );

    for (int i = 0; i < ord->n_traders; i++) {
        trader_t *t = &ord->trader_list[i];
        if (t->pid > 0 && t != tr) { // Disconnected has PID < 0, PID = 0 a root/kernel process
            write(t->exchange_fd, msg, strlen(msg));
            kill(t->pid, SIGUSR1);
        }
    }
}

void notify_amended(orderbook_t *ord, product_t *pr, request_t *req, trader_t *tr)
{
    if (ord == NULL || pr == NULL || req == NULL || tr == NULL) {
        return;
    }

    char msg[MSG_BUFLEN];
    snprintf(msg, MSG_BUFLEN, "AMENDED %d;", req->order_id);
    
    write(tr->exchange_fd, msg, strlen(msg));
    kill(tr->pid, SIGUSR1);

    node_t *order = get_order(pr, req->trader_id, req->order_id);

    snprintf(msg, MSG_BUFLEN, "MARKET %s %s %d %d;", 
        (order->type == BUY_NODE) ? "BUY" : "SELL",
        pr->name,
        req->qty,
        req->price
    );

    for (int i = 0; i < ord->n_traders; i++) {
        trader_t *t = &ord->trader_list[i];
        if (t->pid > 0 && t != tr) {
            write(t->exchange_fd, msg, strlen(msg));
            kill(t->pid, SIGUSR1);
        }
    }
}

void notify_cancelled(orderbook_t *ord, product_t *pr, request_t *req, trader_t *tr)
{
    if (ord == NULL || pr == NULL || req == NULL || tr == NULL) {
        return;
    }

    char msg[MSG_BUFLEN];
    snprintf(msg, MSG_BUFLEN, "CANCELLED %d;", req->order_id);
    
    write(tr->exchange_fd, msg, strlen(msg));
    kill(tr->pid, SIGUSR1);

    node_t *order = get_order(pr, req->trader_id, req->order_id);
    
    snprintf(msg, MSG_BUFLEN, "MARKET %s %s 0 0;", 
        (order->type == BUY_NODE) ? "BUY" : "SELL",
        pr->name
    );

    for (int i = 0; i < ord->n_traders; i++) {
        trader_t *t = &ord->trader_list[i];
        if (t->pid > 0 && t != tr) {
            write(t->exchange_fd, msg, strlen(msg));
            kill(t->pid, SIGUSR1);
        }
    }
}

void notify_invalid(orderbook_t *ord, trader_t *tr)
{
    if (ord == NULL || tr == NULL) {
        return;
    }

    char msg[MSG_BUFLEN];
    snprintf(msg, MSG_BUFLEN, "INVALID;");
    
    write(tr->exchange_fd, msg, strlen(msg));
    kill(tr->pid, SIGUSR1);
}

void notify_fill(trader_t *buyer, trader_t *seller, int buy_order_id, int sell_order_id, int qty)
{
    if (buyer == NULL || seller == NULL) {
        return;
    }

    char msg[MSG_BUFLEN];
    snprintf(msg, MSG_BUFLEN, "FILL %d %d;", buy_order_id, qty);

    if (buyer->pid > 0) {
        write(buyer->exchange_fd, msg, strlen(msg));
        kill(buyer->pid, SIGUSR1);
    }
    

    snprintf(msg, MSG_BUFLEN, "FILL %d %d;", sell_order_id, qty);

    if (seller->pid > 0) {
        write(seller->exchange_fd, msg, strlen(msg));
        kill(seller->pid, SIGUSR1);
    }
}


int get_level(product_t *pr, nodetype_t type)
{
    if (pr == NULL) {
        return 0;
    }

    node_t *sentinel = &pr->orders.sentinel;
    node_t *cursor = sentinel->next;
    
    int levels = 0;

    int price = 0;

    while (cursor != sentinel) {
        if (cursor->type == type && cursor->price != price) {
                price = cursor -> price;
                ++levels;    
        }
        
        cursor = cursor->next;
    }

    return levels;
}

void print_product_orders(product_t *pr)
{
    if (pr == NULL) {
        return;
    }

    pqueue_t *pq = &pr->orders;
    node_t *cursor = pq->sentinel.next;

    while (cursor != &pq->sentinel) {
        nodetype_t type = cursor->type;
        int price = cursor->price;
        int qty = cursor->qty;
        int ord_cnt = 1;

        while ((cursor = cursor->next) != &pq->sentinel) {

            if (cursor->price != price || cursor->type != type) {
                break;
            }

            qty += cursor->qty;
            ord_cnt++;
        }

        printf("[PEX]\t\t%s %d @ $%d (%d %s)\n",
            type == BUY_NODE ? "BUY" : "SELL",
             qty, price, ord_cnt, (ord_cnt == 1) ? "order" : "orders");
    }
}

void print_report(orderbook_t *ord)
{
    if (ord == NULL) {
        return;
    }

    printf("[PEX]\t--ORDERBOOK--\n");
    for (int i = 0; i < ord->n_products; i++) {
        product_t *pr = &ord->product_list[i];
        printf("[PEX]\tProduct: %s; Buy levels: %d; Sell levels: %d\n",
               pr->name, 
               get_level(pr, BUY_NODE), 
               get_level(pr, SELL_NODE)
        );
        print_product_orders(pr);
    }
    
    printf("[PEX]\t--POSITIONS--\n");
    for (int i = 0; i < ord->n_traders; i++) {
        trader_t *t = &ord->trader_list[i];
        printf("[PEX]\tTrader %d: ", t->id);
        for (int j = 0; j < ord->n_products; j++) {
            printf("%s %ld ($%ld)%s", 
                ord->product_list[j].name, 
                t->positions_qty[j], 
                t->positions[j],
                (j == ord->n_products - 1) ? "\n" : ", "
            );
        }
    }
}



void orderbook_free(orderbook_t *ord) {
    if (ord == NULL) {
        return;
    }

    for (int i = 0; i < ord->n_products; i++) {
        product_free(&ord->product_list[i]);
    }
    free(ord->product_list);
    for (int i = 0; i < ord->n_traders; i++) {
        free(ord->trader_list[i].positions);
        free(ord->trader_list[i].positions_qty);
        unlink(ord->trader_list[i].exchange_fifo);
        unlink(ord->trader_list[i].trader_fifo);
    }
    free(ord->trader_list);
}