/*******************************************************************************
 * Test 15
 *
 * Controles d'erreurs
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int sem, i;

        (void)arg;

        assert(screate(-2) == -1);
        assert((sem = screate(2)) >= 0);
        assert(signaln(sem, -4) < 0);
        assert(sreset(sem, -3) == -1);
        assert(scount(sem) == 2);
        assert(signaln(sem, 32760) == 0);
        assert(signaln(sem, 6) == -2);
        assert(scount(sem) == 32762);
        assert(wait(sem) == 0);
        assert(scount(sem) == 32761);
        assert(signaln(sem, 30000) == -2);
        assert(scount(sem) == 32761);
        assert(wait(sem) == 0);
        assert(scount(sem) == 32760);
        assert(signaln(sem, -2) < 0);
        assert(scount(sem) == 32760);
        assert(wait(sem) == 0);
        assert(scount(sem) == 32759);
        assert(signaln(sem, 8) == 0);
        assert(scount(sem) == 32767);
        assert(signaln(sem, 1) == -2);
        assert(scount(sem) == 32767);
        assert(signal(sem) == -2);
        assert(scount(sem) == 32767);
        for (i=0; i<32767; i++) {
                assert(wait(sem) == 0);
        }
        assert(try_wait(sem) == -3);
        assert(scount(sem) == 0);
        assert(sdelete(sem) == 0);
        printf("ok.\n");
}

