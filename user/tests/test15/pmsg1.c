#include "sysapi.h"

int main(void *arg)
{
        int fid1 = (int)arg;

        printf(" 2");
        assert(psend(fid1, 1) == 0);
        assert(psend(fid1, 2) == 0);
        assert(psend(fid1, 3) == 0);
        assert(psend(fid1, 4) == 0);
        assert(psend(fid1, 5) == 457);
        return 1;
}
