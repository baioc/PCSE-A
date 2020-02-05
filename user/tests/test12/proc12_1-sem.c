#include "sysapi.h"

int main(void *arg)
{
        int sem = (int)arg;
        assert(try_wait(sem) == 0);
        assert(try_wait(sem) == -3);
        printf("1");
        assert(wait(sem) == 0);
        printf(" 8");
        assert(wait(sem) == 0);
        printf(" 11");
        exit(1);
        return 0;
}
