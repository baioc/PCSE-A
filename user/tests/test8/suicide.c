#include "sysapi.h"

int main(void *arg)
{
        (void)arg;
        kill(getpid());
        assert(0);
        return 0;
}

