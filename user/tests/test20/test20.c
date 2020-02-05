/*******************************************************************************
 * Test 20
 *
 * Le repas des philosophes.
 ******************************************************************************/
#include "test20.h"

int main(void *arg)
{
        int j, pid;
        struct philo *p;

        (void)arg;

        p = (struct philo*) shm_create("shm_philo");
        assert(p != (void*)0);

        xscreate(&p->mutex_philo); /* semaphore d'exclusion mutuelle */
        xsignal(&p->mutex_philo);
        for (j = 0; j < NR_PHILO; j++) {
                xscreate(p->s + j); /* semaphore de bloquage des philosophes */
                p->f[j] = 0;
                p->bloque[j] = 0;
                p->etat[j] = '-';
        }

        printf("\n");
        pid = start("launch_philo", 4000, 193, 0);
        assert(pid > 0);
        assert(waitpid(pid, 0) == pid);
        printf("\n");
        xsdelete(&p->mutex_philo);
        for (j = 0; j < NR_PHILO; j++) {
                xsdelete(p->s + j);
        }
        shm_release("shm_philo");
}
