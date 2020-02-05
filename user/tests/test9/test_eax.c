#include "sysapi.h"

static unsigned eax = 0xBADB00B5;
static unsigned *it_ok = NULL;

void __test_valid_eax(unsigned a1)
{
        __asm__ __volatile__(
        "   pushl %%eax                 \n" /* Sauver %eax */
        "   movl  %1,       %%eax       \n" /* Attendre un scheduling, ie que *it_ok vaille 1 */
        "0: testl $1,       %2          \n"
        "   jz    0b                    \n"
        "   movl  %%eax,    %0          \n" /* Récupérer la valeur d'%eax après interruption */
        "   popl  %%eax                 \n" /* Restaurer %eax */
        : "=m" (eax)
        : "m" (a1), "m" (*it_ok)
        : "%eax", "memory"
        );
        /* %eax doit avoir conservé sa valeur avant interruption ! */
        assert(eax == a1);
}

int main(void *arg)
{
        (void)arg;
        it_ok = (unsigned*) shm_acquire("test9_shm");
        *it_ok = 0x0u;
        __test_valid_eax(rand());
        shm_release("test9_shm");
        return 0;

}

