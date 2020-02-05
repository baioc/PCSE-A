#include "sysapi.h"
#include "psender.h"

int main(void *arg)
{
        struct psender *ps = NULL;
        int ps_index = (int)arg;
        int msg;
        unsigned i;
        unsigned n;

        ps = shm_acquire("test13_shm");
        assert(ps != NULL);
        n = strlen(ps[ps_index].data);

        for(i = 0; i < n; i++) {
                assert(preceive(ps[ps_index].fid, &msg) == 0);
                assert(msg == ps[ps_index].data[i]);
        }

        shm_release("test13_shm");
        return 0;
}
