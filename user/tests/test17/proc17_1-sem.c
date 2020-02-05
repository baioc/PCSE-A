#include "sysapi.h"
#include "test17-sem.h"

static void buf_send(char x, struct test17_buf_st *st)
{
    assert(wait(st->wsem) == 0);
    assert(wait(st->mutex) == 0);
    st->buf[(st->wpos++) % sizeof(st->buf)] = x;
    assert(signal(st->mutex) == 0);
    assert(signal(st->rsem) == 0);
}

int main(void *arg)
{
        struct test17_buf_st *st = NULL;
        unsigned long long tsc, tsc2;
        int count;

        (void)arg;

        st = (struct test17_buf_st*) shm_acquire("test17_shm");

        __asm__ __volatile__("rdtsc":"=A"(tsc));
        tsc2 = tsc + 1000000000;
        assert(tsc < tsc2);
        do {
                int j;
                for (j=0; j<256; j++) {
                        buf_send((char)j, st);
                }
                count++;
                __asm__ __volatile__("rdtsc":"=A"(tsc));
        } while (tsc < tsc2);
        shm_release("test17_shm");
        return count;
}
