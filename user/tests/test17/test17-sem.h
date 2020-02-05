#ifndef _TEST17_SEM_H_
#define _TEST17_SEM_H_

#include "sysapi.h"

struct test17_buf_st {
    int mutex;
    int wsem;
    unsigned wpos;
    int rsem;
    unsigned rpos;
    char buf[100];
    int received[256];
};

// Increment a variable in a single atomic operation
static __inline__ void atomic_incr(int *atomic)
{
    __asm__ __volatile__("incl %0" : "+m" (*atomic) : : "cc");
}

#endif /* _TEST17_SEM_H_ */
