/*******************************************************************************
 * Test 5
 *
 * Tests de quelques parametres invalides.
 * Certaines interdictions ne sont peut-etre pas dans la spec. Changez les pour
 * faire passer le test correctement.
 ******************************************************************************/

#include "sysapi.h"

int main(void *arg)
{
        int pid1, pid2;
        int r;

        (void)arg;

        // Le processus 0 et la priorite 0 sont des parametres invalides
        assert(kill(0) < 0);
        assert(chprio(getpid(), 0) < 0);
        assert(getprio(getpid()) == 128);
        pid1 = start("no_run", 4000, 64, 0);
        assert(pid1 > 0);
        assert(kill(pid1) == 0);
        assert(kill(pid1) < 0); //pas de kill de zombie
        assert(chprio(pid1, 128) < 0); //changer la priorite d'un zombie
        assert(chprio(pid1, 64) < 0); //changer la priorite d'un zombie
        assert(waitpid(pid1, 0) == pid1);
        assert(waitpid(pid1, 0) < 0);
        pid1 = start("no_run", 4000, 64, 0);
        assert(pid1 > 0);
        pid2 = start("waiter", 4000, 65, (void *)pid1);
        assert(pid2 > 0);
        assert(waitpid(pid2, &r) == pid2);
        assert(r == 1);
        assert(waitpid(pid1, &r) == pid1);
        assert(r == 0);
        printf("ok.\n");
}
