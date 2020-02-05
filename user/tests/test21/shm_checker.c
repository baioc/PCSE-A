/*******************************************************************************
 * Ensimag - Projet Systeme
 * Copyright 2013 - Damien Dejean <dam.dejean@gmail.com>
 *******************************************************************************/

#include "sysapi.h"
#include "test21.h"

int main(void *arg)
{
        (void)arg;
        char *shared_area = NULL;

        shared_area = shm_acquire("test21-shm");
        assert(shared_area != NULL);

        /* Check we get the memory filled by the main process */
        for (int i = 0; i < 4096; i++) {
                if (shared_area[i] != (char)FILL_VALUE) {
                        return -1;
                }
        }

        /*
         * Fill it with something else to let the main process check we success
         * to access it.
         */
        memset(shared_area, 0, 4096);

        return (int)CHECKER_SUCCESS;
}
