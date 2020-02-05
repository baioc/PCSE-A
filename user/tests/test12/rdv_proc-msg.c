#include "sysapi.h"

int main(void *arg)
{
        int fid = (int) arg;
        int msg;
        int count;

        printf(" 2");
        assert(psend(fid, 3) == 0); /* Depose dans le tampon */
        printf(" 3");
        assert((pcount(fid, &count) == 0) && (count == 1));
        assert(psend(fid, 4) == 0); /* Bloque tampon plein */
        printf(" 5");
        assert((pcount(fid, &count) == 0) && (count == 1));
        assert(preceive(fid, &msg) == 0); /* Retire du tampon */
        assert(msg == 4);
        printf(" 6");
        assert(preceive(fid, &msg) == 0); /* Bloque tampon vide. */
        assert(msg == 5);
        printf(" 8");
        assert((pcount(fid, &count) == 0) && (count == 0));
        return 0;
}
