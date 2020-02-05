#include "sysapi.h"

int main(void *arg)
{
        /* arg = priority of this proc. */
        int r;

        assert(getprio(getpid()) == (int) arg);
        printf("1");
        r = chprio(getpid(), 64);
        assert(r == (int) arg);
        printf(" 3");
        return 0;
}

