#include "test11.h"

int main(void *arg)
{
        struct test11_shared *shared = NULL;
        int p = -1;
        int msg;

        (void)arg;
        shared = (struct test11_shared*) shm_acquire("test11_shm");
        assert(shared != NULL);
        p = getprio(getpid());
        assert(p > 0);

        switch (p) {
                case 130:
                        msg = 2;
                        break;
                case 132:
                        msg = 3;
                        break;
                case 131:
                        msg = 4;
                        break;
                case 129:
                        msg = 5;
                        break;
                default:
                        msg = 15;
        }
        printf(" %d", msg);
        xwait(&shared->sem);
        printf(" %d", 139 - p);
        assert(!(shared->in_mutex++));
        chprio(getpid(), 16);
        chprio(getpid(), p);
        shared->in_mutex--;
        xsignal(&shared->sem);
        return 0;

}
