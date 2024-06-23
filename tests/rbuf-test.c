#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "cmocka/cmocka.h"

#include "../modules/rbuf.h"
 
/* rbuf_init constructor testcase. */
static void rbuf_init_sets_default_values(void **state)
{
    rbuf_t rbuf;
    rbuf_init(&rbuf);

    assert_int_equal(rbuf.head, 0);
    assert_int_equal(rbuf.tail, 0);
    assert_int_equal(rbuf.count, 0);
    assert_non_null(rbuf.values);
}

/* peeking into empty buffer returns DEFAULT_SIG_PID. */
static void rbuf_peek_empty(void **state)
{
    rbuf_t rbuf;
    rbuf_init(&rbuf);

    assert_int_equal(rbuf_peek(&rbuf), DEFAULT_SIG_PID);
}
/* After appending, peeking should return appended value. */
static void rbuf_append_into_empty_buffer(void **state)
{
    rbuf_t rbuf;
    rbuf_init(&rbuf);
    int num = 12;

    rbuf_append(&rbuf, num);
    assert_int_equal(rbuf_peek(&rbuf), num);
}
/* 
 * After appending, number set, each added buffer value from head
 * should be equal to the set.
 * Set: [2, 4, 6, 8, 10].
 */
static void rbuf_append_number_set(void **state)
{
    rbuf_t rbuf;
    rbuf_init(&rbuf);
    int num_set[5] = {2, 4, 6, 8, 10};

    for (int i = 0; i < 5; i++) {
        rbuf_append(&rbuf, num_set[i]);
    }

    for (int i = 0; i < 5; i++) {
        assert_int_equal(rbuf.values[rbuf.head + i], num_set[i]);
    }

    assert_int_equal(rbuf.count, 5);
}
/* Appending into a full buffer (count == MAX_EVENTS) should be ignored. */
static void rbuf_append_into_full_buffer(void **state)
{
    rbuf_t rbuf;
    rbuf_init(&rbuf);

    // Exceeds the MAX_EVENTS number of values in container
    for (int i = 0; i < MAX_EVENTS + 1; i++) {
        rbuf_append(&rbuf, i);
    }

    // Test tail value does not overwrite first value
    assert_int_equal(rbuf.values[rbuf.tail], 0);
    assert_int_equal(rbuf.count, MAX_EVENTS);
}
/* Removing from an empty buffer returns DEFAULT_SIG_PID. */
static void rbuf_remove_empty(void **state)
{
    rbuf_t rbuf;
    rbuf_init(&rbuf);
    assert_int_equal(rbuf_remove(&rbuf), DEFAULT_SIG_PID);
}
/* Removing from a buffer returns decrements the count. */
static void rbuf_remove_decrements_count(void **state)
{
    rbuf_t rbuf;
    rbuf_init(&rbuf);

    int num_values = 5;
    int remove_cnt = 1;

    for (int i = 0; i < num_values; i++) {
        rbuf_append(&rbuf, i);
    }

    for (int i = 0; i < remove_cnt; i++) {
        rbuf_remove(&rbuf);
    }
    

    assert_int_equal(rbuf.count, num_values - remove_cnt);
}



int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(rbuf_init_sets_default_values),
        cmocka_unit_test(rbuf_peek_empty),
        cmocka_unit_test(rbuf_append_into_empty_buffer),
        cmocka_unit_test(rbuf_append_number_set),
        cmocka_unit_test(rbuf_append_into_full_buffer),
        cmocka_unit_test(rbuf_remove_empty),
        cmocka_unit_test(rbuf_remove_decrements_count),
    };
 
    return cmocka_run_group_tests(tests, NULL, NULL);
}
