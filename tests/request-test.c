#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "cmocka/cmocka.h"

#include "../modules/request.h"
 

/* WORD COUNT TESTING */

/* Counting from a string with singular word. */
static void count_singular_word(void **state)
{
    const char *str = "blobfish";
    assert_int_equal(count_words(str), 1);
}
/* Counting from a string with multiple words. */
static void count_sentence_string(void **state)
{
    const char *str = "BUY 0 PRODUCT 1 1";
    assert_int_equal(count_words(str), 5);
}
/* Counting from a string multiple words and newline as delimiter. */
static void count_sentence_string_newline(void **state)
{
    const char *str = "This is a\nmulti line sentence";
    assert_int_equal(count_words(str), 6);
}
/* Counting from empty string. */
static void count_empty_string(void **state)
{
    const char *str = "";
    assert_int_equal(count_words(str), 0);
}
/* Counting from a NULL string. */
static void count_null_string(void **state)
{
    const char *str = NULL;
    assert_int_equal(count_words(str), 0);
}


/* GET STRING VECTOR TESTING */

/* String vector with an empty string. */
static void get_empty_string_vector(void **state)
{
    const char *str = "";
    int wrd_cnt = 0;
    char **strvec = get_str_vec(wrd_cnt, str);

    assert_null(strvec);
}
/* String vector with a null string. */
static void get_null_string_vector(void **state)
{
    const char *str = NULL;
    int wrd_cnt = 0;
    char **strvec = get_str_vec(wrd_cnt, str);

    assert_null(strvec);
}
/* String vector from a PEX buy order string format. */
static void get_buy_order_string_vector(void **state)
{
    const char *str = "BUY 0 PRODUCT 1 1;";
    int wrd_cnt = 5;
    char **strvec = get_str_vec(wrd_cnt, str);

    assert_non_null(strvec);

    assert_string_equal(strvec[0], "BUY");
    assert_string_equal(strvec[1], "0");
    assert_string_equal(strvec[2], "PRODUCT");
    assert_string_equal(strvec[3], "1");
    assert_string_equal(strvec[4], "1;");

    for (int i = 0; i < wrd_cnt; i++) {
        free(strvec[i]);
    }
    free(strvec);
}
/* String vector from a PEX amend order string format. */
static void get_amend_order_string_vector(void **state)
{
    const char *str = "AMEND 0 12 140;";
    int wrd_cnt = 4;
    char **strvec = get_str_vec(wrd_cnt, str);

    assert_non_null(strvec);

    assert_string_equal(strvec[0], "AMEND");
    assert_string_equal(strvec[1], "0");
    assert_string_equal(strvec[2], "12");
    assert_string_equal(strvec[3], "140;");

    for (int i = 0; i < wrd_cnt; i++) {
        free(strvec[i]);
    }
    free(strvec);
}
/* 
 * String vector where wrd_cnt > no. of words in a string. 
 * Testcase: wrd_cnt = 5, "A B C D" has word count 4.
 * Expected: strvec: ["A", "B", "C", "D", NULL].
 */
static void get_excessive_wrd_cnt_string_vector(void **state)
{
    const char *str = "A B C D";
    int wrd_cnt = 5;
    char **strvec = get_str_vec(wrd_cnt, str);

    assert_non_null(strvec);

    assert_string_equal(strvec[0], "A");
    assert_string_equal(strvec[1], "B");
    assert_string_equal(strvec[2], "C");
    assert_string_equal(strvec[3], "D");
    assert_null(strvec[4]);

    for (int i = 0; i < wrd_cnt; i++) {
        free(strvec[i]);
    }
    free(strvec);
}
/* 
 * String vector where wrd_cnt > no. of words in a string. 
 * Testcase: wrd_cnt = 3, "A B C D" has word count 4
 * Expected: strvec: ["A", "B", "C"]
 */
static void get_excessive_words_string_vector(void **state)
{
    const char *str = "A B C D";
    int wrd_cnt = 3;
    char **strvec = get_str_vec(wrd_cnt, str);

    assert_non_null(strvec);

    assert_string_equal(strvec[0], "A");
    assert_string_equal(strvec[1], "B");
    assert_string_equal(strvec[2], "C");

    for (int i = 0; i < wrd_cnt; i++) {
        free(strvec[i]);
    }
    free(strvec);
}


/* GET NUMBER TESTING */

/* Get number from string exclusively containing int. */
static void get_number_positive_integer(void **state)
{
    int num = 12;
    char str[11];
    snprintf(str, 11, "%d", num);
    assert_int_equal(get_number(str), num);
}
/* Get number from string exclusively containing negative int. */
static void get_number_negative_integer(void **state)
{
    int num = -12;
    char str[11];
    snprintf(str, 11, "%d", num);

    assert_int_equal(get_number(str), num);
}
/* 
 * Get number from string containing numeric and
 * alphabetical characters. 
 * Example input: "abc1abc".
 * Expected value: -1.
 */
static void get_number_contain_alphanumeric(void **state)
{
    const char *str = "abc1abc";
    assert_int_equal(get_number(str), -1);
}


/* REQUEST PARSE TESTING */

/* Request parsing buy order.
 * Order: T[0] -> "BUY 0 BURGER 10 10;"
 */
static void request_parse_buy_order(void **state)
{
    int trader_id = 0;
    int order_id = 0;
    const char *product_name = "BURGER";
    int qty = 10;
    int price = 10;

    char str[MSG_BUFLEN];
    snprintf(str, MSG_BUFLEN, "BUY %d %s %d %d;", 
        order_id,
        product_name,
        qty,
        price
    );

    request_t req = request_parse(str, trader_id);

    assert_int_equal(req.type, MSG_BUY);
    assert_int_equal(req.trader_id, trader_id);
    assert_int_equal(req.order_id, order_id);
    assert_int_equal(req.qty, qty);
    assert_int_equal(req.price, price);
    assert_string_equal(req.product_name, product_name);
}
/* Request parsing sell order.
 * Order: T[0] -> "SELL 0 WOMBAT 5 15000;"
 */
static void request_parse_sell_order(void **state)
{
    int trader_id = 0;
    int order_id = 0;
    const char *product_name = "WOMBAT";
    int qty = 5;
    int price = 15000;

    char str[MSG_BUFLEN];
    snprintf(str, MSG_BUFLEN, "SELL %d %s %d %d;", 
        order_id,
        product_name,
        qty,
        price
    );

    request_t req = request_parse(str, trader_id);

    assert_int_equal(req.type, MSG_SELL);
    assert_int_equal(req.trader_id, trader_id);
    assert_int_equal(req.order_id, order_id);
    assert_int_equal(req.qty, qty);
    assert_int_equal(req.price, price);
    assert_string_equal(req.product_name, product_name);
}
/* Request parsing sell order.
 * Order: T[0] -> "AMEND 5 10 150;"
 */
static void request_parse_amend_order(void **state)
{
    int trader_id = 0;
    int order_id = 0;
    int qty = 10;
    int price = 150;

    char str[MSG_BUFLEN];
    snprintf(str, MSG_BUFLEN, "AMEND %d %d %d;", 
        order_id,
        qty,
        price
    );

    request_t req = request_parse(str, trader_id);

    assert_int_equal(req.type, MSG_AMEND);
    assert_int_equal(req.trader_id, trader_id);
    assert_int_equal(req.order_id, order_id);
    assert_int_equal(req.qty, qty);
    assert_int_equal(req.price, price);
    assert_string_equal(req.product_name, "");
}
/* Request parsing sell order.
 * Order: T[0] -> "CANCEL 0;"
 */
static void request_parse_cancel_order(void **state)
{
    int trader_id = 0;
    int order_id = 0;

    char str[MSG_BUFLEN];
    snprintf(str, MSG_BUFLEN, "CANCEL %d;", 
        order_id
    );

    request_t req = request_parse(str, trader_id);

    assert_int_equal(req.type, MSG_CANCEL);
    assert_int_equal(req.trader_id, trader_id);
    assert_int_equal(req.order_id, order_id);
}
/* Request parsing invalid order (out of bounds qty).
 * Qty must be in range [1, 999999].
 * Order: T[0] -> "BUY 0 BURGER 0 10;"
 */
static void request_parse_order_invalid_zero_qty(void **state)
{
    int trader_id = 0;
    int order_id = 0;
    const char *product_name = "BURGER";
    int qty = 0;
    int price = 10;

    char str[MSG_BUFLEN];
    snprintf(str, MSG_BUFLEN, "BUY %d %s %d %d;", 
        order_id,
        product_name,
        qty,
        price
    );

    request_t req = request_parse(str, trader_id);

    assert_not_in_range(qty, 1, 999999);
    assert_int_equal(req.type, MSG_INVALID);
}
/* Request parsing invalid order (out of bounds price).
 * Price must be in range [1, 999999].
 * Order: T[0] -> "BUY 0 BURGER 10 0;"
 */
static void request_parse_order_invalid_zero_price(void **state)
{
    int trader_id = 0;
    int order_id = 0;
    const char *product_name = "BURGER";
    int qty = 10;
    int price = 0;

    char str[MSG_BUFLEN];
    snprintf(str, MSG_BUFLEN, "BUY %d %s %d %d;", 
        order_id,
        product_name,
        qty,
        price
    );

    request_t req = request_parse(str, trader_id);

    assert_not_in_range(price, 1, 999999);
    assert_int_equal(req.type, MSG_INVALID);
}
/* Request parsing invalid order (Unknown message).
 * Order: T[0] -> "I'm sorry autocorrect, I never meant to write BIGGER;"
 */
static void request_parse_order_invalid_message(void **state)
{
    int trader_id = 0;
    const char *str = "brap brap brap stutututu;";

    request_t req = request_parse(str, trader_id);

    assert_int_equal(req.type, MSG_INVALID);
}


 
int main(void)
{
    const struct CMUnitTest tests[] = {
        /* word counting */
        cmocka_unit_test(count_singular_word),
        cmocka_unit_test(count_sentence_string),
        cmocka_unit_test(count_sentence_string_newline),
        cmocka_unit_test(count_empty_string),
        cmocka_unit_test(count_null_string),
        /* get string vector */
        cmocka_unit_test(get_empty_string_vector),
        cmocka_unit_test(get_null_string_vector),
        cmocka_unit_test(get_buy_order_string_vector),
        cmocka_unit_test(get_amend_order_string_vector),
        cmocka_unit_test(get_excessive_wrd_cnt_string_vector),
        cmocka_unit_test(get_excessive_words_string_vector),
        /* get number */
        cmocka_unit_test(get_number_positive_integer),
        cmocka_unit_test(get_number_negative_integer),
        cmocka_unit_test(get_number_contain_alphanumeric),
        /* Request Parsing */
        cmocka_unit_test(request_parse_buy_order),
        cmocka_unit_test(request_parse_sell_order),
        cmocka_unit_test(request_parse_amend_order),
        cmocka_unit_test(request_parse_cancel_order),
        cmocka_unit_test(request_parse_order_invalid_zero_qty),
        cmocka_unit_test(request_parse_order_invalid_zero_price),
        cmocka_unit_test(request_parse_order_invalid_message),

    };
 
    return cmocka_run_group_tests(tests, NULL, NULL);
}
