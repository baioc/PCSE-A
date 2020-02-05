#include "sysapi.h"

int main(void *arg)
{
        int sem = (int)arg;
        printf("1");
        assert(wait(sem) == 0);
        printf(" 5");
        assert(wait(sem) == 0);
        printf(" 11");
        exit(1);
}

