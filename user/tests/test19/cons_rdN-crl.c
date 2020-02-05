#include "sysapi.h"

int main(void *arg)
{
        unsigned long i;
        char buf[101];

        (void)arg;

        i = cons_read(buf, 100);
        buf[i] = 0;
        printf("%d : %s\n", 133 - getprio(getpid()), buf);
        return 0;
}
