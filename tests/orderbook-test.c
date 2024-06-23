#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "cmocka/cmocka.h"
 
#include "../modules/orderbook.h"

/* 
 * Returns pointer to a product_t struct present in orderbook.
 * The key is determined by string value.
 */
static void str_get_product_valid(void **state)
{
    orderbook_t ord = {
        .fees = 0,
        .n_traders = 0,
        .n_products = 3,
        .trader_list = NULL,
        .product_list = malloc(ord.n_products * sizeof(product_t))
    };

    product_init(&ord.product_list[0], "APPLE", 0);
    product_init(&ord.product_list[1], "ORANGE", 1);
    product_init(&ord.product_list[2], "BANANA", 2);

    assert_ptr_equal(&ord.product_list[0], str_get_product(&ord, "APPLE"));
    assert_ptr_equal(&ord.product_list[1], str_get_product(&ord, "ORANGE"));
    assert_ptr_equal(&ord.product_list[2], str_get_product(&ord, "BANANA"));

    orderbook_free(&ord);
}
/* 
 * Returns NULL if a product_t struct with key value is NOT
 * present in orderbook. The key is determined by string value.
 */
static void str_get_product_invalid(void **state)
{
    orderbook_t ord = {
        .fees = 0,
        .n_traders = 0,
        .n_products = 3,
        .trader_list = NULL,
        .product_list = malloc(ord.n_products * sizeof(product_t))
    };

    product_init(&ord.product_list[0], "APPLE", 0);
    product_init(&ord.product_list[1], "ORANGE", 1);
    product_init(&ord.product_list[2], "BANANA", 2);

    assert_null(str_get_product(&ord, "STRAWBERRY"));

    orderbook_free(&ord);
}
/* 
 * Returns pointer to a product_t struct present in orderbook if there
 * exists an order in the product's orderlist matched with the id pairs.
 * The key is determined by trader_id and order_id.
 */
static void id_get_product_valid(void **state)
{
    orderbook_t ord = {
        .fees = 0,
        .n_traders = 0,
        .n_products = 3,
        .trader_list = NULL,
        .product_list = malloc(ord.n_products * sizeof(product_t))
    };

    product_init(&ord.product_list[0], "APPLE", 0);
    product_init(&ord.product_list[1], "ORANGE", 1);
    product_init(&ord.product_list[2], "BANANA", 2);

    product_t *pr = str_get_product(&ord, "APPLE");

    int trader_id = 0;
    int order_id = 0;
    node_t *buy_order = init_node(BUY_NODE, trader_id, order_id, 12, 12);

    pq_enqueue_buy(&pr->orders, buy_order);

    assert_ptr_equal(&ord.product_list[0], id_get_product(&ord, trader_id, order_id));

    orderbook_free(&ord);
}
/* 
 * Returns NULL if there does not exist an order in any product's orderlist 
 * matched with the id pairs.
 * The key is determined by trader_id and order_id.
 */
static void id_get_product_invalid(void **state)
{
    orderbook_t ord = {
        .fees = 0,
        .n_traders = 0,
        .n_products = 3,
        .trader_list = NULL,
        .product_list = malloc(ord.n_products * sizeof(product_t))
    };

    product_init(&ord.product_list[0], "APPLE", 0);
    product_init(&ord.product_list[1], "ORANGE", 1);
    product_init(&ord.product_list[2], "BANANA", 2);

    int trader_id = 0;
    int order_id = 0;

    // No enqueue
    assert_null(id_get_product(&ord, trader_id, order_id));

    orderbook_free(&ord);
}
/* 
 * Returns pointer to a trader_t struct present in orderbook.
 * The key is determined by trader_id value.
 */
static void get_trader_valid(void **state)
{
    orderbook_t ord = {
        .fees = 0,
        .n_traders = 3,
        .n_products = 0,
        .trader_list = malloc(ord.n_traders * sizeof(trader_t)),
        .product_list = NULL
    };

    trader_init(&ord.trader_list[0], 0, ord.n_products);
    trader_init(&ord.trader_list[1], 1, ord.n_products);
    trader_init(&ord.trader_list[2], 2, ord.n_products);

    assert_ptr_equal(&ord.trader_list[0], get_trader(&ord, 0));
    assert_ptr_equal(&ord.trader_list[1], get_trader(&ord, 1));
    assert_ptr_equal(&ord.trader_list[2], get_trader(&ord, 2));

    orderbook_free(&ord);
}
/* 
 * Returns NULL if a trader_t struct is NOT present in orderbook.
 * The key is determined by trader_id value.
 */
static void get_trader_invalid(void **state)
{
    orderbook_t ord = {
        .fees = 0,
        .n_traders = 3,
        .n_products = 0,
        .trader_list = malloc(ord.n_traders * sizeof(trader_t)),
        .product_list = NULL
    };

    trader_init(&ord.trader_list[0], 0, ord.n_products);
    trader_init(&ord.trader_list[1], 1, ord.n_products);
    trader_init(&ord.trader_list[2], 2, ord.n_products);

    assert_null(get_trader(&ord, -1));
    assert_null(get_trader(&ord, ord.n_traders + 1));
    assert_null(get_trader(NULL, 12));

    orderbook_free(&ord);
}

// get_order is a pq_get wrapper.


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(str_get_product_valid),
        cmocka_unit_test(str_get_product_invalid),
        cmocka_unit_test(id_get_product_valid),
        cmocka_unit_test(str_get_product_invalid),
        cmocka_unit_test(get_trader_valid),
        cmocka_unit_test(get_trader_invalid),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
