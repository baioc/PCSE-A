#include "sysapi.h"

int main(void *arg)
{
        int pid1;
        int p = 192;
        int r;

        (void)arg;

        assert(getprio(getpid()) == 128);
        pid1 = start("prio4", 4000, p, (void *) p);
        assert(pid1 > 0);
        printf(" 2");
        r = chprio(getpid(), 32);
        assert(r == 128);
        printf(" 4");
        r = chprio(getpid(), 128);
        assert(r == 32);
        printf(" 5");
        assert(waitpid(pid1, 0) == pid1);
        printf(" 6");

        assert(getprio(getpid()) == 128);
        pid1 = start("prio5", 4000, p, (void *) p);
        assert(pid1 > 0);
        printf(" 8");
        r = kill(pid1);
        assert(r == 0);
        assert(waitpid(pid1, 0) == pid1);
        printf(" 9");
        r = chprio(getpid(), 32);
        assert(r == 128);
        printf(" 10");
        r = chprio(getpid(), 128);
        assert(r == 32);
        printf(" 11.\n");
}


