/*******************************************************************************
 * Test 17
 *
 * Un exemple de producteur/consommateur
 * On peut aussi faire sans operation atomique
 ******************************************************************************/

#include "sysapi.h"
#include "test17-sem.h"

int main(void *arg)
{
        int pid[6];
        int i;
        struct test17_buf_st *st = NULL;
        int count = 0;

        (void)arg;
        st = (struct test17_buf_st*) shm_create("test17_shm");
        assert(st != NULL);

        assert(getprio(getpid()) == 128);
        st->mutex = screate(1);
        assert(st->mutex >= 0);
        st->wsem = screate(100);
        assert(st->wsem >= 0);
        st->wpos = 0;
        st->rsem = screate(0);
        assert(st->rsem >= 0);
        st->rpos = 0;
        for (i=0; i<256; i++) {
                st->received[i] = 0;
        }
        for (i=0; i<3; i++) {
                pid[i] = start("proc17_1", 4000, 129, &st);
                assert(pid[i] > 0);
        }
        for (i=3; i<6; i++) {
                pid[i] = start("proc17_2", 4000, 129, &st);
                assert(pid[i] > 0);
        }
        for (i=0; i<3; i++) {
                int ret;
                assert(waitpid(pid[i], &ret) == pid[i]);
                count += ret;
        }
        assert(scount(st->rsem) == 0xfffd);
        for (i=3; i<6; i++) {
                int ret;
                assert(kill(pid[i]) == 0);
                assert(waitpid(pid[i], &ret) == pid[i]);
        }
        assert(scount(st->mutex) == 1);
        assert(scount(st->wsem) == 100);
        assert(scount(st->rsem) == 0);
        assert(sdelete(st->mutex) == 0);
        assert(sdelete(st->wsem) == 0);
        assert(sdelete(st->rsem) == 0);
        for (i=0; i<256; i++) {
                int n = st->received[i];
                if (n != count) {
                        printf("st->received[%d] == %d, count == %d\n", i, n, count);
                        assert(n == count);
                }
        }
        printf("ok (%d chars sent).\n", count * 256);
}
