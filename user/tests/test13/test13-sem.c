/*******************************************************************************
 * Test 13
 *
 * sreset, sdelete
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
        pid1 = start("proc13_1", 4000, 129, (void *)sem);
        assert(pid1 > 0);
        printf(" 2");
        pid2 = start("proc13_2", 4000, 127, (void *)sem);
        assert(pid2 > 0);
        pid3 = start("proc13_3", 4000, 130, (void *)sem);
        assert(pid3 > 0);
        printf(" 4");
        assert(chprio(getpid(), 126) == 128);
        printf(" 6");
        assert(chprio(getpid(), 128) == 126);
        assert(sreset(sem, 1) == 0);
        printf(" 10");
        assert(chprio(getpid(), 126) == 128);
        assert(chprio(getpid(), 128) == 126);
        assert(sdelete(sem) == 0);
        printf(" 14");
        assert(waitpid(pid1, &ret) == pid1);
        assert(ret == 1);
        assert(waitpid(-1, &ret) == pid3);
        assert(ret == 3);
        assert(waitpid(-1, &ret) == pid2);
        assert(ret == 2);
        assert(signal(sem) == -1);
        assert(scount(sem) == -1);
        assert(sdelete(sem) == -1);
        printf(" 16.\n");
        return 0;
}
