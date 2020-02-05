#include "sysapi.h"

int main(void *arg)
{
        int sem = (int)arg;
        printf(" 3");
        assert(wait(sem) == -4);
        printf(" 7");
        assert(wait(sem) == 0);
        printf(" 8");
        assert(wait(sem) == -3);
        printf(" 12");
        assert(wait(sem) == -1);
        exit(3);
        assert(!"Should not arrive here !");
        while(1);
}
