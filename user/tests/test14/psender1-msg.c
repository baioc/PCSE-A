#include "sysapi.h"

int main(void *arg)
{
        int fid1 = (int)arg;
        int fid2;
        int msg;

        printf(" 2");
        assert(preceive(fid1, &fid2) == 0);
        assert(psend(fid1, fid2) == 0);
        fid2 -= 42;
        assert(psend(fid1, 1) == 0);
        assert(psend(fid1, 2) == 0);
        assert(psend(fid1, 3) == 0);
        assert(psend(fid1, 4) == 0);
        assert(psend(fid1, 5) < 0);
        printf(" 6");
        assert(psend(fid1, 12) < 0);
        printf(" 9");
        assert(psend(fid1, 14) < 0);
        assert(preceive(fid2, &msg) < 0);
        printf(" 12");
        assert(preceive(fid2, &msg) < 0);
        assert(preceive(fid2, &msg) < 0);
        return 0;
}
