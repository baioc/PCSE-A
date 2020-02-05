/*******************************************************************************
 * Test 16
 *
 * Test sur des files de diverses tailles et test d'endurance
 ******************************************************************************/

#include "sysapi.h"
#include "test16-msg.h"

#define NB_PROCS 10

int main(void *arg)
{
        int i, count, fid, pid;
        struct tst16 *p = NULL;
        int pids[2 * NB_PROCS];

        (void)arg;
        p = (struct tst16*) shm_create("test16_shm");
        assert(p != NULL);

        assert(getprio(getpid()) == 128);
        for (count = 1; count <= 100; count++) {
                fid = pcreate(count);
                assert(fid >= 0);
                p->count = count;
                p->fid = fid;
                pid = start("proc16_1", 2000, 128, 0);
                assert(pid > 0);
                for (i=0; i<=count; i++) {
                        assert(psend(fid, i) == 0);
                        test_it();
                }
                assert(waitpid(pid, 0) == pid);
                assert(pdelete(fid) == 0);
        }

        p->count = 20000;
        fid = pcreate(50);
        assert(fid >= 0);
        p->fid = fid;
        for (i = 0; i< NB_PROCS; i++) {
                pid = start("proc16_2", 2000, 127, 0);
                assert(pid > 0);
                pids[i] = pid;
        }
        for (i=0; i < NB_PROCS; i++) {
                pid = start("proc16_3", 2000, 127, 0);
                assert(pid > 0);
                pids[NB_PROCS + i] = pid;
        }
        for (i=0; i < 2 * NB_PROCS; i++) {
                assert(waitpid(pids[i], 0) == pids[i]);
        }
        assert(pcount(fid, &count) == 0);
        assert(count == 0);
        assert(pdelete(fid) == 0);

        shm_release("test16_shm");
        printf("ok.\n");
        return 0;
}
