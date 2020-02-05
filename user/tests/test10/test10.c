/*******************************************************************************
 * Test 10
 * Test l'utilisation des semaphores ou des files de messages selon le sujet.
 *******************************************************************************/
#include "sysapi.h"

#if defined WITH_SEM
/* Test d'utilisation d'un semaphore comme simple compteur. */
int main(void *arg)
{
        int sem1;
        (void)arg;
        sem1 = screate(2);
        assert(sem1 >= 0);
        assert(scount(sem1) == 2);
        assert(signal(sem1) == 0);
        assert(scount(sem1) == 3);
        assert(signaln(sem1, 2) == 0);
        assert(scount(sem1) == 5);
        assert(wait(sem1) == 0);
        assert(scount(sem1) == 4);
        assert(sreset(sem1, 7) == 0);
        assert(scount(sem1) == 7);
        assert(sdelete(sem1) == 0);
        printf("ok.\n");
        return 0;
}

#elif defined WITH_MSG
/* Test d'utilisation d'une file comme espace de stockage temporaire. */

static void write(int fid, const char *buf, unsigned long len)
{
        unsigned long i;
        for (i=0; i<len; i++) {
                assert(psend(fid, buf[i]) == 0);
        }
}

static void read(int fid, char *buf, unsigned long len)
{
        unsigned long i;
        for (i=0; i<len; i++) {
                int msg;
                assert(preceive(fid, &msg) == 0);
                buf[i] = (char)msg;
        }
}
int main(void *arg)
{
        int fid;
        const char *str = "abcde";
        unsigned long len = strlen(str);
        char buf[10];

        (void)arg;

        printf("1");
        assert((fid = pcreate(5)) >= 0);
        write(fid, str, len);
        printf(" 2");
        read(fid, buf, len);
        buf[len] = 0;
        assert(strcmp(str, buf) == 0);
        assert(pdelete(fid) == 0);
        printf(" 3.\n");
        return 0;
}

#else
# error "WITH_SEM" ou "WITH_MSG" doit être définie.
#endif

