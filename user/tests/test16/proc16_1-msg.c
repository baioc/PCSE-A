#include "sysapi.h"
#include "test16-msg.h"

int main(void *arg)
{
        struct tst16 *p = NULL;
        int i, msg;

        (void)arg;
        p = shm_acquire("test16_shm");
        assert(p != NULL);

        for (i = 0; i <= p->count; i++) {
                assert(preceive(p->fid, &msg) == 0);
                assert(msg == i);
                test_it();
        }
        shm_release("test16_shm");
        return 0;
}
