/*******************************************************************************
 * Test 12
 *
 * Deblocages par signaln, atomicite.
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int sem;
        int pid1, pid2, pid3;
        int ret;

        (void)arg;

        assert(getprio(getpid()) == 128);
        assert((sem = screate(1)) >= 0);
        pid1 = start("proc12_1", 4000, 129, (void *)sem);
        assert(pid1 > 0);
        printf(" 2");
        pid2 = start("proc12_2", 4000, 127, (void *)sem);
        assert(pid2 > 0);
        pid3 = start("proc12_3", 4000, 130, (void *)sem);
        assert(pid3 > 0);
        printf(" 4");
        assert(chprio(getpid(), 126) == 128);
        printf(" 6");
        assert(chprio(getpid(), 128) == 126);
        assert(signaln(sem, 2) == 0);
        assert(signaln(sem, 1) == 0);
        assert(signaln(sem, 4) == 0);
        assert(waitpid(pid1, &ret) == pid1);
        assert(ret == 1);
        assert(waitpid(-1, &ret) == pid3);
        assert(ret == 0);
        assert(scount(sem) == 1);
        assert(sdelete(sem) == 0);
        printf(" 12");
        assert(waitpid(-1, &ret) == pid2);
        assert(ret == 2);
        printf(" 14.\n");
        return 0;
}
