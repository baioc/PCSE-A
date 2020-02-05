/*******************************************************************************
 * Ensimag - Projet Systeme
 * Copyright 2013 - Damien Dejean <dam.dejean@gmail.com>
 *******************************************************************************/

#include "sysapi.h"
#include "test22.h"

int main(void *arg)
{
        unsigned *shared_area = NULL;

        assert(arg == NULL);

        shared_area = shm_acquire("test22-shm");
        assert(shared_area != NULL);

        /* The parent process should have filled the shared area */
        assert(*shared_area == MAGIC_COOKIE);

        /* Write in the memory to force w flag in the TLB */
        *shared_area = MAGIC_COOKIE;

        /*
         * Try to fool the kernel: shm memory is usually done by dynamic memory
         * mapping, if TLB is not cleaned up after unmap, we should keep the
         * access to shared memory even if the entry is no more valid in page
         * directory/table !
         */

        /* Unmap */
        printf("%s", "  Unmapping shared area ... ");
        shm_release("test22-shm");
        printf("%s\n", "OK");

        /* Try a read */
        printf("%s", "  Try a read ... ");
        assert(*shared_area == MAGIC_COOKIE);
        printf("%s\n", " done, this is wrong, I should have been killed ! ");

        /* Try a write */
        printf("%s", "  Try a write ... ");
        *shared_area = 0xDEADB00B;
        printf("%s\n", " done, this is wrong, I should have been killed ! ");

        /*
         * Page is not really unmapped until the TLB is cleaned up ! Fix your
         * kernel !
         */

        return (int)MALICIOUS_SUCCESS;
}
