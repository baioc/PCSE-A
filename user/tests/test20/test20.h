#ifndef _TEST20_H_
#define _TEST20_H_

#include "sysapi.h"

struct philo {
        char f[NR_PHILO]; /* tableau des fourchettes, contient soit 1 soit 0 selon si elle
                             est utilisee ou non */
        char bloque[NR_PHILO]; /* memorise l'etat du philosophe, contient 1 ou 0 selon que le philosophe
                                  est en attente d'une fourchette ou non */
        /* Padding pour satisfaire un compilateur strict. */
        char padding[sizeof(int) - (NR_PHILO * 2) % sizeof(int)];
        union sem mutex_philo; /* exclusion mutuelle */
        union sem s[NR_PHILO]; /* un semaphore par philosophe */
        int etat[NR_PHILO];
};

#endif
