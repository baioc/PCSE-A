#include "sysapi.h"

int main(void * arg)
{
        int sem = (int)arg;
        printf(" 5");
        assert(wait(sem) == 0);
        printf(" 13");
        return 2;
}
