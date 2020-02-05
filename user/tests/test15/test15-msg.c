/*******************************************************************************
 * Test 15
 *
 * Tuer des processus en attente sur file
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int pid1, pid2, fid1;
        int msg;
        int count = 1;
        int r = 1;

        (void)arg;

        assert((fid1 = pcreate(3)) >= 0);
        printf("1");
        assert(getprio(getpid()) == 128);
        pid1 = start("pmsg1", 4000, 131, (void *)fid1);
        assert(pid1 > 0);
        pid2 = start("pmsg2", 4000, 130, (void *)fid1);
        assert(pid2 > 0);

        assert((preceive(fid1, &msg) == 0) && (msg == 1));
        assert(kill(pid1) == 0);
        assert(kill(pid1) < 0);
        assert((preceive(fid1, &msg) == 0) && (msg == 2));
        assert(kill(pid2) == 0);
        assert(kill(pid2) < 0);
        assert(preceive(fid1, &msg) == 0);
        assert(msg == 3);
        assert(preceive(fid1, &msg) == 0);
        assert(msg == 4);
        assert(preceive(fid1, &msg) == 0);
        assert(msg == 6);
        assert(pcount(fid1, &count) == 0);
        assert(count == 0);
        assert(waitpid(pid1, &r) == pid1);
        assert(r == 0);
        r = 1;
        assert(waitpid(-1, &r) == pid2);
        assert(r == 0);
        assert(pdelete(fid1) == 0);
        assert(pdelete(fid1) < 0);
        printf(" 4.\n");
        return 0;
}
