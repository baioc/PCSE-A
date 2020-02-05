#include "sysapi.h"

int main(void *arg)
{
        int fid1 = (int)arg;
        int fid2;
        int msg;

        printf(" 3");
        assert(preceive(fid1, &fid2) == 0);
        fid2 -= 42;
        assert(psend(fid1, 6) < 0);
        printf(" 5");
        assert(psend(fid1, 7) == 0);
        assert(psend(fid1, 8) == 0);
        assert(psend(fid1, 9) == 0);
        assert(psend(fid1, 10) == 0);
        assert(psend(fid1, 11) < 0);
        printf(" 8");
        assert(psend(fid1, 13) < 0);
        assert((preceive(fid2, &msg) == 0) && (msg == 15));
        assert(preceive(fid2, &msg) < 0);
        printf(" 11");
        assert(preceive(fid2, &msg) < 0);
        assert(preceive(fid2, &msg) < 0);
        return 0;
}
