#include "request.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int count_words(const char *str)
{
    if (str == NULL) {
        return 0;
    }

    int cnt = 0;
    int wordlen = 0;
    for (int i = 0; str[i]; i++) {
        if (!isspace(str[i])) {
            ++wordlen;
        } else if (wordlen > 0 && isspace(str[i])) {
            ++cnt;
            wordlen = 0;
        }
    }
    return (wordlen > 0) ? cnt + 1 : cnt;    
}

char **get_str_vec(int wrd_cnt, const char *buf)
{
    if (wrd_cnt < 1 || buf == NULL) {
        return NULL;
    }

    char **str_vec = calloc(wrd_cnt, sizeof(char *));
    int arg_idx = 0;
    char *start, *parse, *tok; 
    start = parse = strdup(buf);

    while ((tok = strsep(&parse, " ")) != NULL && arg_idx < wrd_cnt) {
        if (tok[0] != 0) {
            size_t len = strlen(tok);
            (str_vec)[arg_idx] = malloc((len + 1) * sizeof(char));
            memcpy((str_vec)[arg_idx++], tok, len + 1);
        }
    }

    free(start);

    return str_vec;
}

int get_number(const char *str)
{
    if (str == NULL) {
        return -1;
    }

    char *sptr;
    int n = strtol(str, &sptr, 10);
    if (sptr == str) {
        return -1;
    }
    return n;
}

request_t request_parse(const char *str, int trader_id)
{
    request_t ret = {0};
    int wrd_cnt = count_words(str);
    char **str_arr = get_str_vec(wrd_cnt, str);

    /* TRADER ID */
    ret.trader_id = trader_id;

    /* TYPE */
    if (wrd_cnt == 5 && strcmp(str_arr[0], "BUY") == 0) {
        ret.type = MSG_BUY;
    } else if (wrd_cnt == 5 && strcmp(str_arr[0], "SELL") == 0) {
        ret.type = MSG_SELL;
    } else if (wrd_cnt == 4 && strcmp(str_arr[0], "AMEND") == 0) {
        ret.type = MSG_AMEND;
    } else if (wrd_cnt == 2 && strcmp(str_arr[0], "CANCEL") == 0) {
        ret.type = MSG_CANCEL;
    } else {
        ret.type = MSG_INVALID;
    }

    /* ORDER ID */
    if (ret.type == MSG_INVALID) {
        ret.order_id = INT32_MIN;
    } else {
        ret.order_id = get_number(str_arr[1]);
    }

    /* PRODUCT NAME */
    memset(ret.product_name, 0, PRODUCT_BUFLEN);
    if (ret.type == MSG_BUY || ret.type == MSG_SELL) {
        strncpy(ret.product_name, str_arr[2], PRODUCT_BUFLEN - 1);
    }

    switch (ret.type) {
        case MSG_BUY:   // fall through
        case MSG_SELL:
            ret.qty = get_number(str_arr[3]);
            ret.price = get_number(str_arr[4]);
            break;
        case MSG_AMEND:
            ret.qty = get_number(str_arr[2]);
            ret.price = get_number(str_arr[3]);
            break;
        default:                                    // CANCEL + INVALID
            ret.qty = INT32_MIN;
            ret.price = INT32_MIN;
    }

    /* Parameter Checking */
    if ((ret.type == MSG_BUY || ret.type == MSG_SELL || ret.type == MSG_AMEND)
        && (ret.order_id < 0 || ret.order_id > 999999 || 
            ret.qty < 1 || ret.qty > 999999 || 
            ret.price < 1 || ret.price > 999999)) {

        ret.type = MSG_INVALID;

    } else if (ret.type == MSG_CANCEL 
        && (ret.order_id < 0 || ret.order_id > 999999)) {

        ret.type = MSG_INVALID;
        
    }

    for (int i = 0; i < wrd_cnt; i++) {
        free(str_arr[i]);
    }
    free(str_arr);

    return ret;
}