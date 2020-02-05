#include "sysapi.h"

int main(void *arg)
{
        int sem = (int)arg;
        printf(" 3");
        assert(wait(sem) == 0);
        printf(" 7");
        assert(wait(sem) == 0);
        printf(" 9");
        assert(wait(sem) == 0);
        printf(" 10");
        kill(getpid());
        assert(!"Should not arrive here !");
        while(1);
        return 0;
}
