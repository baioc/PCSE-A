#include "sysapi.h"

int main(void *arg)
{
        int sem = (int)arg;
        assert(try_wait(sem) == 0);
        assert(try_wait(sem) == -3);
        printf("1");
        assert(wait(sem) == -4);
        printf(" 9");
        assert(wait(sem) == -3);
        printf(" 13");
        assert(wait(sem) == -1);

        exit(1);
}
