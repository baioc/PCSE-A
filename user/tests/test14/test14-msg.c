/*******************************************************************************
 * Test 14
 *
 * Tests de preset et pdelete
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int pid1, pid2;
        int fid1 = pcreate(3);
        int fid2 = pcreate(3);
        int msg;

        (void)arg;

        /* Bravo si vous n'etes pas tombe dans le piege. */
        assert(pcreate(1073741827) < 0);

        printf("1");
        assert(getprio(getpid()) == 128);
        assert(fid1 >= 0);
        assert(psend(fid1, fid2 + 42) == 0);
        pid1 = start("psender1", 4000, 131, (void *)fid1);
        pid2 = start("psender2", 4000, 130, (void *)fid1);
        assert((preceive(fid1, &msg) == 0) && (msg == 1));
        assert(chprio(pid2, 132) == 130);
        printf(" 4");
        assert(preset(fid1) == 0);
        assert((preceive(fid1, &msg) == 0) && (msg == 7));
        printf(" 7");
        assert(pdelete(fid1) == 0);
        printf(" 10");
        assert(psend(fid2, 15) == 0);
        assert(preset(fid2) == 0);
        printf(" 13");
        assert(pdelete(fid2) == 0);
        assert(pdelete(fid2) < 0);
        assert(waitpid(pid2, 0) == pid2); //XXX assert(waitpid(-1, 0) == pid2); ???
        assert(waitpid(-1, 0) == pid1);
        printf(".\n");
}
