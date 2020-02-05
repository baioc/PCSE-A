#include "sysapi.h"

static unsigned long test16_1(void)
{
        unsigned long long tsc, tsc1, tsc2;
        unsigned long count = 0;

        __asm__ __volatile__("rdtsc":"=A"(tsc1));
        tsc2 = tsc1 + 1000000000;
        assert(tsc1 < tsc2);
        do {
                unsigned i;
                test_it();
                for (i=0; i<100; i++) {
                        int sem1 = screate(2);
                        int sem2 = screate(1);
                        assert(sem1 >= 0);
                        assert(sem2 >= 0);
                        assert(sem1 != sem2);
                        assert(sdelete(sem1) == 0);
                        assert(sdelete(sem2) == 0);
                }
                __asm__ __volatile__("rdtsc":"=A"(tsc));
                count += 2 * i;

        } while (tsc < tsc2);
        return (unsigned long)div64(tsc - tsc1, count, 0);
}

int main(void *arg)
{
        int sems[NBSEMS];
        int i;
        unsigned long c1, c2;
        unsigned long long seed;

        (void)arg;

        c1 = test16_1();
        printf("%lu ", c1);
        __asm__ __volatile__("rdtsc":"=A"(seed));
        setSeed(seed);
        for (i=0; i<NBSEMS; i++) {
                int sem = screate(randShort());
                if (sem < 0) assert(!"*** Increase the semaphore capacity of your system to NBSEMS to pass this test. ***");
                sems[i] = sem;
        }
        if (screate(0) >= 0) assert(!"*** Decrease the semaphore capacity of your system to NBSEMS to pass this test. ***");
        assert(sdelete(sems[NBSEMS/3]) == 0);
        assert(sdelete(sems[(NBSEMS/3)*2]) == 0);
        c2 = test16_1();
        printf("%lu ", c2);
        setSeed(seed);
        for (i=0; i<NBSEMS; i++) {
                short randVal = randShort();
                if ((i != (NBSEMS/3)) && (i != (2*(NBSEMS/3)))) {
                        assert(scount(sems[i]) == randVal);
                        assert(sdelete(sems[i]) == 0);
                }
        }
        if (c2 < 2 * c1)
                printf("ok.\n");
        else
                printf("Bad algorithm complexity in semaphore allocation.\n");
        return 0;
}


