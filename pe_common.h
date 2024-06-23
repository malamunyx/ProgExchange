#ifndef PE_COMMON_H
#define PE_COMMON_H

#ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
#endif

#ifndef _POSIX_SOURCE
    #define _POSIX_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>

#define FIFO_NAMELEN 30
#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1

#define PRODUCT_BUFLEN 20
#define MSG_BUFLEN 64

#define DEFAULT_SIG_PID -1

#define MAX_EVENTS 32

#endif
