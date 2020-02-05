#include "test20.h"

int main(void *arg)
{
        int i, pid;

        (void)arg;

        for (i = 0; i < NR_PHILO; i++) {
                pid = start("philosophe", 4000, 192, (void *) i);
                assert(pid > 0);
        }
        return 0;
}
