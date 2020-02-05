/*******************************************************************************
 * Test 1
 *
 * Demarrage de processus avec passage de parametre
 * Terminaison normale avec valeur de retour
 * Attente de terminaison (cas fils avant pere et cas pere avant fils)
 ******************************************************************************/

#include "test1.h"
#include "sysapi.h"

int main(void *arg)
{
        int pid1;
        int r;
        int rval;

        (void)arg;

        pid1 = start("dummy1", 4000, 192, (void *) DUMMY_VAL);
        assert(pid1 > 0);
        printf(" 2");
        r = waitpid(pid1, &rval);
        assert(r == pid1);
        assert(rval == 3);
        printf(" 3");
        pid1 = start("dummy2", 4000, 100, (void *) (DUMMY_VAL + 1));
        assert(pid1 > 0);
        printf(" 4");
        r = waitpid(pid1, &rval);
        assert(r == pid1);
        assert(rval == 4);
        printf(" 6.\n");
        return 0;
}

