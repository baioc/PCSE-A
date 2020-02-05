/*******************************************************************************
 * Test 14
 *
 * chprio et kill de processus bloque sur semaphore
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int sem;
        int pid1, pid2;

        (void)arg;

        assert(getprio(getpid()) == 128);
        assert((sem = screate(0)) >= 0);
        pid1 = start("proc14_1", 4000, 129, (void *)sem);
        assert(pid1 > 0);
        printf(" 2");
        pid2 = start("proc14_2", 4000, 130, (void *)sem);
        assert(pid2 > 0);
        printf(" 4");
        assert(chprio(pid1, 131) == 129);
        assert(signal(sem) == 0);
        printf(" 6");
        assert(chprio(pid1, 127) == 131);
        assert(signal(sem) == 0);
        printf(" 8");
        assert(signaln(sem, 2) == 0);
        printf(" 10");
        assert(waitpid(pid1, 0) == pid1);
        assert(scount(sem) == 0xffff);
        assert(kill(pid2) == 0);
        assert(getprio(pid2) < 0);
        assert(chprio(pid2, 129) < 0);
        assert(scount(sem) == 0);
        assert(signal(sem) == 0);
        assert(scount(sem) == 1);
        assert(sdelete(sem) == 0);
        assert(waitpid(-1, 0) == pid2);
        printf(" 12.\n");
        return 0;
}
