#include "rbuf.h"

void rbuf_init(rbuf_t *r) {
    r->head = 0;
    r->tail = 0;
    r->count = 0;
}

int rbuf_peek(rbuf_t *r)
{
    if (r->count > 0) {
        return r->values[r->head];
    }

    return DEFAULT_SIG_PID;
}

void rbuf_append(rbuf_t *r, int val)
{
    if (r->count == MAX_EVENTS) {
        return;
    }

    r->values[r->tail++] = val;
    r->tail %= MAX_EVENTS;
    ++(r->count);
}



int rbuf_remove(rbuf_t *r)
{
    if (r->count > 0) {
        int val = r->values[r->head++];
        r->head %= MAX_EVENTS;
        --(r->count);
        return val;
    }

    return DEFAULT_SIG_PID;
}