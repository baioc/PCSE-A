/*******************************************************************************
 * Ensimag - Projet Systeme
 * Copyright 2013 - Damien Dejean <dam.dejean@gmail.com>
 * Test 22
 *
 * Checks the state of CPU's Translation Lookaside Buffer when the kernel does
 * dynamic memory mapping for a process.
 ******************************************************************************/

#include "sysapi.h"
#include "test22.h"

int main(void *arg)
{
        (void)arg;
        int pagefault_pid = -1;
        int pagefault_ret = -1;
        unsigned *shared_area = NULL;
        int malicious_pid = -1;
        int malicious_ret = -1;

        /*
         * Check page fault handling.
         */
        printf("\n%s\n", "Test 22: checking page fault handling...");
        pagefault_pid = start("pagefault", 4000, getprio(getpid()) - 1, NULL);

        waitpid(pagefault_pid, &pagefault_ret);

        /* "pagefault" should have been killed */
        switch (pagefault_ret) {
                case 0:
                        printf("-> %s\n-> %s\n", "\"pagefault\" process killed.", "TEST PASSED");
                        break;

                case (int)MALICIOUS_SUCCESS:
                        printf("-> %s\n-> %s\n", "\"pagefault\" process should not ends correctly.", "TEST FAILED");
                        break;
                default:
                        printf("-> %s\n-> %s\n", "unexpected return value for \"pagefault\" process. Check waitpid and kill behaviors", "TEST FAILED");
        }


        /*
         * Check virtual memory mappings consistency.
         */
        shared_area = shm_create("test22-shm");
        assert(shared_area != NULL);

        /* The malicious process will check this value */
        *shared_area = MAGIC_COOKIE;

        printf("%s\n", "Test 22: checking shared memory mappings ...");
        malicious_pid = start("malicious", 4000, getprio(getpid()) - 1, NULL);

        waitpid(malicious_pid, &malicious_ret);

        /* "malicious" should have been killed */
        switch (malicious_ret) {
                case 0:
                        printf("-> %s\n-> %s\n", "\"malicious\" process killed.", "TEST PASSED");
                        break;

                case (int)MALICIOUS_SUCCESS:
                        printf("-> %s\n-> %s\n", "\"malicious\" process should not ends correctly.", "TEST FAILED");
                        break;
                default:
                        printf("-> %s\n-> %s\n", "unexpected return value for \"malicious\" process. Check waitpid and kill behaviors", "TEST FAILED");
        }

        /* and should not have been able to modify shared memory */
        if (*shared_area != MAGIC_COOKIE) {
                printf("-> %s\n-> %s\n", "\"malicious\" have been able to write unmapped memory.", "TEST FAILED");
        }

        shm_release("test22-shm");
        while(1);
        return 0;
}
