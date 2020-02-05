#include "sysapi.h"

int main(void *arg)
{
        int sem = (int)arg;
        printf(" 5");
        assert(wait(sem) == -4);
        printf(" 11");
        assert(wait(sem) == -3);
        printf(" 15");
        assert(wait(sem) == -1);

        return 2;
}
