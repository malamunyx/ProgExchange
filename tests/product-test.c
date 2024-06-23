#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "cmocka/cmocka.h"

#include "../modules/product.h"


/* pq_init constructor testcase. */
static void pq_init_sets_default_values(void **state)
{
    pqueue_t pq;

    pq_init(&pq);

    assert_ptr_equal(pq.sentinel.next, &pq.sentinel);
    assert_ptr_equal(pq.sentinel.prev, &pq.sentinel);
    assert_int_equal(pq.sentinel.trader_id, INT32_MIN);
    assert_int_equal(pq.sentinel.order_id, INT32_MIN);
    assert_int_equal(pq.sentinel.qty, INT32_MIN);
    assert_int_equal(pq.sentinel.price, INT32_MIN);
}
/* product_init constructor testcase. */
static void product_init_sets_default_values(void **state)
{
    product_t pr;

    const char *pr_name = "blobfish";
    int pr_id = 0;
    product_init(&pr, pr_name, pr_id);

    assert_string_equal(pr.name, pr_name);
    assert_int_equal(pr.product_id, pr_id);
}
/* init_node, buy node (order) */
static void init_node_buy_type(void **state)
{
    nodetype_t type = BUY_NODE;
    int trader_id = 10;
    int order_id = 10;
    int qty = 10;
    int price = 10;

    node_t *n = init_node(type, trader_id, order_id, qty, price);

    assert_non_null(n);
    assert_int_equal(n->type, type);
    assert_int_equal(n->trader_id, trader_id);
    assert_int_equal(n->order_id, order_id);
    assert_int_equal(n->qty, qty);
    assert_int_equal(n->price, price);

    free(n);
}
/* init_node, sell node (order) */
static void init_node_sell_type(void **state)
{
    nodetype_t type = SELL_NODE;
    int trader_id = 10;
    int order_id = 10;
    int qty = 10;
    int price = 10;

    node_t *n = init_node(type, trader_id, order_id, qty, price);

    assert_non_null(n);
    assert_int_equal(n->type, type);
    assert_int_equal(n->trader_id, trader_id);
    assert_int_equal(n->order_id, order_id);
    assert_int_equal(n->qty, qty);
    assert_int_equal(n->price, price);

    free(n);
}

/* get node from empty list returns null */
static void pq_get_empty_list_returns_null(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    int trader_id = 0;
    int order_id = 0;

    assert_null(pq_get(&pq, trader_id, order_id));
}
/* get existent node in list return address */
static void pq_get_valid_node(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    int qty = 10;
    int price = 10;

    node_t *a = init_node(BUY_NODE, 0, 0, qty, price);
    node_t *b = init_node(BUY_NODE, 1, 1, qty, price);
    node_t *c = init_node(BUY_NODE, 2, 2, qty, price);

    pq_enqueue_buy(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_buy(&pq, c);

    // Assert correct
    assert_ptr_equal(pq_get(&pq, 0, 0), a);
    assert_ptr_equal(pq_get(&pq, 1, 1), b);
    assert_ptr_equal(pq_get(&pq, 2, 2), c);

    free(a);
    free(b);
    free(c);
}
/* get nonexistent node in list returns null */
static void pq_get_invalid_node_returns_null(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    int qty = 10;
    int price = 10;

    node_t *a = init_node(BUY_NODE, 0, 0, qty, price);
    node_t *b = init_node(BUY_NODE, 1, 1, qty, price);
    node_t *c = init_node(BUY_NODE, 2, 2, qty, price);

    pq_enqueue_buy(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_buy(&pq, c);

    // Assert correct
    assert_null(pq_get(&pq, 0, 1));

    free(a);
    free(b);
    free(c);
}
/* 
 * Buy orders are match-traversed from the front, hence frontwards is BUY PRIORITY.
 * pq_enqueue_buy maintains correct time priority ordering.
 * Earlier orders have priority.
 * <FRONT> [t = 1] BUY 10 10, [t = 2] BUY 10 10, [t = 3] BUY 10 10 <REAR>
 */
static void pq_enqueue_buy_maintains_time_priority(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    int qty = 10;
    int price = 10;

    node_t *a = init_node(BUY_NODE, 0, 0, qty, price);
    node_t *b = init_node(BUY_NODE, 1, 1, qty, price);
    node_t *c = init_node(BUY_NODE, 2, 2, qty, price);

    pq_enqueue_buy(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_buy(&pq, c);

    node_t *cursor = pq.sentinel.next;
    assert_ptr_equal(a, cursor);

    cursor = cursor->next;
    assert_ptr_equal(b, cursor);

    cursor = cursor->next;
    assert_ptr_equal(c, cursor);

    free(a);
    free(b);
    free(c);
}
/* 
 * Buy orders are match-traversed from the front, hence frontwards is BUY PRIORITY.
 * pq_enqueue_buy maintains correct price priority ordering.
 * Higher buy prices, higher priority.
 * <FRONT> [t = 3] BUY 15 15, [t = 2] BUY 12 12, [t = 1] BUY 10 10 <REAR>
 */
static void pq_enqueue_buy_maintains_price_priority(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    node_t *a = init_node(BUY_NODE, 0, 0, 10, 10);
    node_t *b = init_node(BUY_NODE, 0, 1, 12, 12);
    node_t *c = init_node(BUY_NODE, 0, 2, 15, 15);

    pq_enqueue_buy(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_buy(&pq, c);

    node_t *cursor = pq.sentinel.next;
    assert_ptr_equal(c, cursor);

    cursor = cursor->next;
    assert_ptr_equal(b, cursor);

    cursor = cursor->next;
    assert_ptr_equal(a, cursor);

    free(a);
    free(b);
    free(c);
}
/* 
 * Buy orders are match-traversed from the front, hence frontwards is BUY PRIORITY.
 * pq_enqueue_buy maintains correct price-time priority ordering.
 * Higher buy prices, higher priority. If equal prices, earlier order gets priority.
 * <FRONT> [t = 3] BUY 15 15, [t = 1] BUY 10 10, [t = 2] BUY 10 10 <REAR>
 */
static void pq_enqueue_buy_maintains_order_price_priority(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    node_t *a = init_node(BUY_NODE, 0, 0, 10, 10);
    node_t *b = init_node(BUY_NODE, 0, 1, 10, 10);
    node_t *c = init_node(BUY_NODE, 0, 2, 15, 15);

    pq_enqueue_buy(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_buy(&pq, c);

    node_t *cursor = pq.sentinel.next;
    assert_ptr_equal(c, cursor);

    cursor = cursor->next;
    assert_ptr_equal(a, cursor);

    cursor = cursor->next;
    assert_ptr_equal(b, cursor);

    free(a);
    free(b);
    free(c);
}
/* 
 * Sell orders are match-traversed from the rear, hence rearwards is SELL PRIORITY.
 * pq_enqueue_sell maintains correct time priority ordering.
 * Earlier orders have priority.
 * <FRONT> [t = 3] SELL 10 10, [t = 2] SELL 10 10, [t = 1] SELL 10 10 <REAR>
 */
static void pq_enqueue_sell_maintains_time_priority(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    int qty = 10;
    int price = 10;

    node_t *a = init_node(SELL_NODE, 0, 0, qty, price);
    node_t *b = init_node(SELL_NODE, 1, 1, qty, price);
    node_t *c = init_node(SELL_NODE, 2, 2, qty, price);

    pq_enqueue_sell(&pq, a);
    pq_enqueue_sell(&pq, b);
    pq_enqueue_sell(&pq, c);

    node_t *cursor = pq.sentinel.prev;
    assert_ptr_equal(a, cursor);

    cursor = cursor->prev;
    assert_ptr_equal(b, cursor);

    cursor = cursor->prev;
    assert_ptr_equal(c, cursor);

    free(a);
    free(b);
    free(c);
}
/* 
 * Sell orders are match-traversed from the rear, hence rearwards is SELL PRIORITY.
 * pq_enqueue_sell maintains correct time priority ordering.
 * Lower sell prices, higher priority.
 * <FRONT> [t = 3] SELL 15 15, [t = 2] SELL 12 12, [t = 1] SELL 10 10 <REAR>
 */
static void pq_enqueue_sell_maintains_price_priority(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    node_t *a = init_node(SELL_NODE, 0, 0, 10, 10);
    node_t *b = init_node(SELL_NODE, 0, 1, 12, 12);
    node_t *c = init_node(SELL_NODE, 0, 2, 15, 15);

    pq_enqueue_sell(&pq, a);
    pq_enqueue_sell(&pq, b);
    pq_enqueue_sell(&pq, c);

    node_t *cursor = pq.sentinel.prev;
    assert_ptr_equal(a, cursor);

    cursor = cursor->prev;
    assert_ptr_equal(b, cursor);

    cursor = cursor->prev;
    assert_ptr_equal(c, cursor);

    free(a);
    free(b);
    free(c);
}
/* 
 * Sell orders are match-traversed from the rear, hence rearwards is SELL PRIORITY.
 * pq_enqueue_sell maintains correct price-time priority ordering.
 * Lower sell prices, higher priority. If equal prices, earlier order gets priority.
 * <FRONT> [t = 1] SELL 15 15, [t = 3] SELL 10 10, [t = 2] SELL 10 10 <REAR>
 */
static void pq_enqueue_sell_maintains_order_price_priority(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    node_t *a = init_node(SELL_NODE, 0, 0, 15, 15);
    node_t *b = init_node(SELL_NODE, 0, 1, 10, 10);
    node_t *c = init_node(SELL_NODE, 0, 2, 10, 10);

    pq_enqueue_sell(&pq, a);
    pq_enqueue_sell(&pq, b);
    pq_enqueue_sell(&pq, c);

    

    node_t *cursor = pq.sentinel.prev;
    assert_ptr_equal(b, cursor);

    cursor = cursor->prev;
    assert_ptr_equal(c, cursor);

    cursor = cursor->prev;
    assert_ptr_equal(a, cursor);

    free(a);
    free(b);
    free(c);
}
/* 
 * Contains both BUY/SELL NODES. Front -> Rear.
 * <FRONT>  [t = 5] SELL 35 35, [t = 3] SELL 25 25, [t = 1] SELL 25 25
 *          [t = 2] BUY 15 15,  [t = 4] BUY 10 10,  [t = 6] BUY 10 10   <REAR>
 */
static void pq_enqueue_mix(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    // Selling T: 5, 3, 1 -> e, c, a
    node_t *e = init_node(SELL_NODE, 0, 5, 35, 35);
    node_t *c = init_node(SELL_NODE, 0, 3, 25, 25);
    node_t *a = init_node(SELL_NODE, 0, 1, 25, 25);


    // Buying T: 6, 4, 2 -> f, d, b
    node_t *f = init_node(BUY_NODE, 0, 6, 10, 10);
    node_t *d = init_node(BUY_NODE, 0, 4, 10, 10);
    node_t *b = init_node(BUY_NODE, 0, 2, 15, 15);

    pq_enqueue_sell(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_sell(&pq, c);
    pq_enqueue_buy(&pq, d);
    pq_enqueue_sell(&pq, e);
    pq_enqueue_buy(&pq, f);

    node_t *cursor = pq.sentinel.next;
    assert_ptr_equal(e, cursor);

    cursor = cursor->next;
    assert_ptr_equal(c, cursor);

    cursor = cursor->next;
    assert_ptr_equal(a, cursor);

    cursor = cursor->next;
    assert_ptr_equal(b, cursor);

    cursor = cursor->next;
    assert_ptr_equal(d, cursor);

    cursor = cursor->next;
    assert_ptr_equal(f, cursor);

    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
}
/* 
 * Contains both BUY/SELL NODES. Front -> Rear.
 * <FRONT>  [t = 5] SELL 35 35, [t = 3] SELL 35 35, [t = 1] SELL 25 25
 *          [t = 2] BUY 15 15,  [t = 4] BUY 15 15,  [t = 6] BUY 10 10   <REAR>
 */
static void pq_enqueue_mix_2(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    // Selling T: 5, 3, 1 -> e, c, a
    node_t *e = init_node(SELL_NODE, 0, 5, 35, 35);
    node_t *c = init_node(SELL_NODE, 0, 3, 35, 35);
    node_t *a = init_node(SELL_NODE, 0, 1, 25, 25);


    // Buying T: 6, 4, 2 -> f, d, b
    node_t *f = init_node(BUY_NODE, 0, 6, 10, 10);
    node_t *d = init_node(BUY_NODE, 0, 4, 15, 15);
    node_t *b = init_node(BUY_NODE, 0, 2, 15, 15);

    pq_enqueue_sell(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_sell(&pq, c);
    pq_enqueue_buy(&pq, d);
    pq_enqueue_sell(&pq, e);
    pq_enqueue_buy(&pq, f);

    node_t *cursor = pq.sentinel.next;
    assert_ptr_equal(e, cursor);

    cursor = cursor->next;
    assert_ptr_equal(c, cursor);

    cursor = cursor->next;
    assert_ptr_equal(a, cursor);

    cursor = cursor->next;
    assert_ptr_equal(b, cursor);

    cursor = cursor->next;
    assert_ptr_equal(d, cursor);

    cursor = cursor->next;
    assert_ptr_equal(f, cursor);

    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
}
/* pq_remove node from an order list containing 1 node. */
static void pq_remove_list_size_1(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    int qty = 10;
    int price = 10;

    node_t *a = init_node(BUY_NODE, 0, 0, qty, price);

    pq_enqueue_buy(&pq, a);

    pq_remove(&pq, a);

    assert_ptr_equal(&pq.sentinel, pq.sentinel.next);
    assert_ptr_equal(&pq.sentinel, pq.sentinel.prev);

    free(a);
}
/* pq_remove node from an order list containing 3 node. */
static void pq_remove_list_size_3(void **state)
{
    pqueue_t pq;
    pq_init(&pq);

    int qty = 10;
    int price = 10;

    node_t *a = init_node(BUY_NODE, 0, 0, qty, price);
    node_t *b = init_node(BUY_NODE, 1, 1, qty, price);
    node_t *c = init_node(BUY_NODE, 2, 2, qty, price);

    pq_enqueue_buy(&pq, a);
    pq_enqueue_buy(&pq, b);
    pq_enqueue_buy(&pq, c);

    pq_remove(&pq, b);

    node_t *cursor = pq.sentinel.next;
    assert_ptr_equal(a, cursor);

    cursor = cursor->next;
    assert_ptr_equal(c, cursor);

    cursor = pq.sentinel.prev;
    assert_ptr_equal(c, cursor);

    cursor = cursor->prev;
    assert_ptr_equal(a, cursor);


    free(a);
    free(b);
    free(c);
}



int main(void)
{
    const struct CMUnitTest tests[] = {
        /* init (constructors) */
        cmocka_unit_test(pq_init_sets_default_values),
        cmocka_unit_test(product_init_sets_default_values),
        cmocka_unit_test(init_node_buy_type),
        cmocka_unit_test(init_node_sell_type),
        /* pq_get */
        cmocka_unit_test(pq_get_empty_list_returns_null),
        cmocka_unit_test(pq_get_valid_node),
        cmocka_unit_test(pq_get_invalid_node_returns_null),
        /* pq_enqueue */
        cmocka_unit_test(pq_enqueue_buy_maintains_time_priority),
        cmocka_unit_test(pq_enqueue_buy_maintains_price_priority),
        cmocka_unit_test(pq_enqueue_buy_maintains_order_price_priority),
        cmocka_unit_test(pq_enqueue_sell_maintains_time_priority),
        cmocka_unit_test(pq_enqueue_sell_maintains_price_priority),
        cmocka_unit_test(pq_enqueue_sell_maintains_order_price_priority),
        cmocka_unit_test(pq_enqueue_mix),
        cmocka_unit_test(pq_enqueue_mix_2),
        /* pq_remove */
        cmocka_unit_test(pq_remove_list_size_1),
        cmocka_unit_test(pq_remove_list_size_3),
    };
 
    return cmocka_run_group_tests(tests, NULL, NULL);
}