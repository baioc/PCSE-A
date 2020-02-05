#include "test20.h"

static void affiche_etat(struct philo *p)
{
        int i;
        printf("%c", 13);
        for (i=0; i<NR_PHILO; i++) {
                unsigned long c;
                switch (p->etat[i]) {
                        case 'm':
                                c = 2;
                                break;
                        default:
                                c = 4;
                }
                (void)c;
                printf("%c", p->etat[i]);
        }
}

static void waitloop(void)
{
        int j;
        for (j = 0; j < 5000; j++) {
                int l;
                test_it();
                for (l = 0; l < 5000; l++);
        }
}

static void penser(struct philo *p, long i)
{
        xwait(&p->mutex_philo); /* DEBUT SC */
        p->etat[i] = 'p';
        affiche_etat(p);
        xsignal(&p->mutex_philo); /* Fin SC */
        waitloop();
        xwait(&p->mutex_philo); /* DEBUT SC */
        p->etat[i] = '-';
        affiche_etat(p);
        xsignal(&p->mutex_philo); /* Fin SC */
}

static void manger(struct philo *p, long i)
{
        xwait(&p->mutex_philo); /* DEBUT SC */
        p->etat[i] = 'm';
        affiche_etat(p);
        xsignal(&p->mutex_philo); /* Fin SC */
        waitloop();
        xwait(&p->mutex_philo); /* DEBUT SC */
        p->etat[i] = '-';
        affiche_etat(p);
        xsignal(&p->mutex_philo); /* Fin SC */
}

static int test(struct philo *p, int i)
{
        /* les fourchettes du philosophe i sont elles libres ? */
        return ((!p->f[i] && (!p->f[(i + 1) % NR_PHILO])));
}

static void prendre_fourchettes(struct philo *p, int i)
{
        /* le philosophe i prend des fourchettes */

        xwait(&p->mutex_philo); /* Debut SC */

        if (test(p, i)) {  /* on tente de prendre 2 fourchette */
                p->f[i] = 1;
                p->f[(i + 1) % NR_PHILO] = 1;
                xsignal(&p->s[i]);
        } else
                p->bloque[i] = 1;

        xsignal(&p->mutex_philo); /* FIN SC */
        xwait(&p->s[i]); /* on attend au cas o on ne puisse pas prendre 2 fourchettes */
}

static void poser_fourchettes(struct philo *p, int i)
{

        xwait(&p->mutex_philo); /* DEBUT SC */

        if ((p->bloque[(i + NR_PHILO - 1) % NR_PHILO]) && (!p->f[(i + NR_PHILO - 1) % NR_PHILO])) {
                p->f[(i + NR_PHILO - 1) % NR_PHILO] = 1;
                p->bloque[(i + NR_PHILO - 1) % NR_PHILO] = 0;
                xsignal(&p->s[(i + NR_PHILO - 1) % NR_PHILO]);
        } else
                p->f[i] = 0;

        if ((p->bloque[(i + 1) % NR_PHILO]) && (!p->f[(i + 2) % NR_PHILO])) {
                p->f[(i + 2) % NR_PHILO] = 1;
                p->bloque[(i + 1) % NR_PHILO] = 0;
                xsignal(&p->s[(i + 1) % NR_PHILO]);
        } else
                p->f[(i + 1) % NR_PHILO] = 0;

        xsignal(&p->mutex_philo); /* Fin SC */
}


int main(void *arg)
{
        /* comportement d'un seul philosophe */
        int i = (int) arg;
        int k;
        struct philo *p;

        p = shm_acquire("shm_philo");
        assert(p != (void*)0);

        for (k = 0; k < 6; k++) {
                prendre_fourchettes(p, i); /* prend 2 fourchettes ou se bloque */
                manger(p, i); /* le philosophe mange */
                poser_fourchettes(p, i); /* pose 2 fourchettes */
                penser(p, i); /* le philosophe pense */
        }
        xwait(&p->mutex_philo); /* DEBUT SC */
        p->etat[i] = '-';
        affiche_etat(p);
        xsignal(&p->mutex_philo); /* Fin SC */
        shm_release("shm_philo");
        return 0;
}

