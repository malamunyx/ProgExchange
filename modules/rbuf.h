#ifndef RBUF_H
#define RBUF_H

#include "../pe_common.h"

/* rbuf struct (PID Ring Buffer) */

typedef struct rbuf {
    int values[MAX_EVENTS];
    int head;
    int tail;
    int count;
} rbuf_t;



/*
 * Initialises ring buffer queue struct.
 */
void rbuf_init(rbuf_t *r);
/*
 * Get the priority value in the ring buffer.
 */
int rbuf_peek(rbuf_t *r);
/*
 * Enqueue/append value to rbuf. If buffer is full, append is ignored. 
 */
void rbuf_append(rbuf_t *r, int val);
/*
 * Remove the priority value in the ring buffer.
 */
int rbuf_remove(rbuf_t *r);

#endif