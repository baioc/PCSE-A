/*******************************************************************************
 * Test 12
 *
 * Tests de rendez-vous sur une file de taille 1.
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int fid;
        int pid;
        int msg;
        int count;

        (void)arg;

        assert(getprio(getpid()) == 128);
        assert((fid = pcreate(1)) >= 0);
        printf("1");
        pid = start("rdv_proc", 4000, 130, (void *)fid);
        assert(pid > 0);
        printf(" 4");
        assert((pcount(fid, &count) == 0) && (count == 2));
        assert(preceive(fid, &msg) == 0); /* Retire du tampon et debloque un emetteur. */
        assert(msg == 3);
        printf(" 7");
        assert((pcount(fid, &count) == 0) && (count == -1));
        assert(psend(fid, 5) == 0); /* Pose dans le tampon. */
        printf(" 9");
        assert(psend(fid, 6) == 0); /* Pose dans le tampon. */
        assert(preceive(fid, &msg) == 0); /* Retire du tampon. */
        assert(msg == 6);
        assert(pdelete(fid) == 0);
        assert(psend(fid, 2) < 0);
        assert(preceive(fid, &msg) < 0);
        assert(waitpid(-1, 0) == pid);
        printf(" 10.\n");
        return 0;
}
