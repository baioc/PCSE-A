/*******************************************************************************
 * Test 9
 *
 * Test de la sauvegarde des registres dans les appels systeme et interruptions
 ******************************************************************************/

#include "sysapi.h"

static unsigned int ebp_before = 0x12345678u;
static unsigned int esp_before = 0x87654321u;
static unsigned int ebp_after  = 0xDEADBEEFu;
static unsigned int esp_after  = 0xDEADFACEu;
static unsigned int eax        = 0xBADB00B5u;
static unsigned int ebx        = 0xF0F0F0F0u;
static unsigned int edi        = 0x0F0F0F0Fu;
static unsigned int esi        = 0xABCDEFABu;

__asm__(
"       .data                   \n"
"nothing:                       \n"
"       .string \"nothing\"     \n"
"       .previous               \n"
);

void __test_valid_regs1(unsigned a1, unsigned a2, unsigned a3)
{
        __asm__ __volatile__(
        /* Assigner des valeurs connues aux registres */
        "movl   %8,     %%ebx           \n"
        "movl   %9,     %%edi           \n"
        "movl   %10,    %%esi           \n"
        "movl   %%ebp,  %0              \n"
        "movl   %%esp,  %1              \n"

        /* Démarrer le processus "nothing" */
        "pushl  $0                      \n"
        "pushl  $192                    \n"
        "pushl  $4000                   \n"
        "pushl  $nothing                \n"
        "call   start                   \n"
        "addl   $16,    %%esp           \n"
        "movl   %%eax,  %2              \n"

        /* Sauver les registres */
        "movl %%ebx,    %3              \n"
        "movl %%edi,    %4              \n"
        "movl %%esi,    %5              \n"
        "movl %%ebp,    %6              \n"
        "movl %%esp,    %7              \n"

        /* Registres de sortie */
        : "=m" (ebp_before), "=m" (esp_before),
          "=m" (eax),        "=m" (ebx),
          "=m" (edi),        "=m" (esi),
          "=m" (ebp_after),  "=m" (esp_after)
        /* Registres en entrée */
        : "m" (a1),
          "m" (a2),
          "m" (a3)
        /* Registres utilisés par ce bloc ASM*/
        : "eax", "ebx", "edi", "esi", "memory"
        );

        /* On attend le processus nothing dont le pid est dans eax */
        assert(waitpid((int)eax, NULL) == (int)eax);

        /* Vérifier les valeurs des registres après l'appel */
        assert(ebx == a1);
        assert(edi == a2);
        assert(esi == a3);
        assert(ebp_before == ebp_after);
        assert(esp_before == esp_after);
}

int main(void *arg)
{
        int i;
        int pid;
        volatile unsigned *it_ok = NULL;

        (void)arg;
        it_ok = (unsigned*) shm_create("test9_shm");
        assert(it_ok != NULL);
        assert(getprio(getpid()) == 128);
        printf("1");

        for (i = 0; i < 1000; i++) {
                __test_valid_regs1(rand(), rand(), rand());
        }
        printf(" 2");

        /* Test de la cohérence de tous les registres */
        for (i = 0; i < 100; i++) {
                *it_ok = 1;
                pid = start("test_regs2", 4000, 128, 0);
                assert(pid > 0);
                while (*it_ok != 0);
                *it_ok = 1;
                assert(waitpid(pid, 0) == pid);
        }
        printf(" 3");

        /* Test de la cohérence de %eax */
        for (i = 0; i < 100; i++) {
                *it_ok = 1;
                pid = start("test_eax", 4000, 128, 0);
                assert(pid > 0);
                while (*it_ok != 0);
                *it_ok = 1;
                assert(waitpid(pid, 0) == pid);
        }
        printf(" 4.\n");

        shm_release("test9_shm");
        return 0;
}
