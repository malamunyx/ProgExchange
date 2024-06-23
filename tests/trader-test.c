#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "cmocka/cmocka.h"
 
#include "../modules/trader.h"

/* TRADER INIT (CONSTRUCTOR) TESTING */

/* trader_init constructor testcase */
static void trader_init_sets_default_values(void **state)
{
    int id = 1;
    int product_qty = 2;
    char ex_fifo[FIFO_NAMELEN];
    char tr_fifo[FIFO_NAMELEN];

    /* Expected FIFO naming */
    snprintf(ex_fifo, FIFO_NAMELEN, FIFO_EXCHANGE, id);
    snprintf(tr_fifo, FIFO_NAMELEN, FIFO_TRADER, id);

    trader_t tr;
    trader_init(&tr, id, product_qty);

    /* Expected default values */
    assert_int_equal(tr.pid, INT32_MIN);
    assert_int_equal(tr.id, id);
    assert_int_equal(tr.order_id, 0);
    assert_int_equal(tr.exchange_fd, INT32_MIN);
    assert_int_equal(tr.trader_fd, INT32_MIN);
    assert_int_equal(tr.product_qty, product_qty);
    
    assert_string_equal(tr.exchange_fifo, ex_fifo);
    assert_string_equal(tr.trader_fifo, tr_fifo);

    /* Successful dynamic allocation */
    assert_non_null(tr.positions);
    assert_non_null(tr.positions_qty);
    
    free(tr.positions);
    free(tr.positions_qty);
}
 
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(trader_init_sets_default_values),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
