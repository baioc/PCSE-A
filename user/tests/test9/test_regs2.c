#include "sysapi.h"

static unsigned ebx = 1;
static unsigned ecx = 2;
static unsigned edx = 3;
static unsigned edi = 4;
static unsigned esi = 5;
static unsigned ebp_before = 0x12345678u;
static unsigned esp_before = 0x98765432u;
static unsigned ebp_after = 0xCAFE0101u;
static unsigned esp_after = 0x1010CAFEu;
static volatile unsigned *it_ok = NULL;

void __test_valid_regs2(unsigned a1,
                        unsigned a2,
                        unsigned a3,
                        unsigned a4,
                        unsigned a5)
{
        /* Initialise les registres avec des valeurs spéciales */
        __asm__ __volatile__(
        "movl %2,       %%ebx    \n"
        "movl %3,       %%ecx    \n"
        "movl %4,       %%edx    \n"
        "movl %5,       %%edi    \n"
        "movl %6,       %%esi    \n"
        /* Sauve le "stack pointer" et le "frame pointer" */
        "movl %%ebp,     %0      \n"
        "movl %%esp,     %1      \n"
        : "=m" (ebp_before), "=m" (esp_before)
        : "m" (a1), "m" (a2), "m" (a3), "m" (a4), "m" (a5)
        : "%ebx","%ecx", "%edx", "%edi", "%esi", "memory"
        );

        /* Attendre au moins un scheduling (ie des interruptions) */
        while (*it_ok == 0);

        /* Sauver les valeurs des registres */
        __asm__ __volatile__(
        "movl %%ebp,    %0      \n"
        "movl %%esp,    %1      \n"
        "movl %%ebx,    %2      \n"
        "movl %%ecx,    %3      \n"
        "movl %%edx,    %4      \n"
        "movl %%edi,    %5      \n"
        "movl %%esi,    %6      \n"
        : "=m" (ebp_after), "=m" (esp_after), "=m" (ebx), "=m" (ecx), "=m" (edx), "=m" (edi), "=m" (esi)
        : /* No input registers */
        : "memory"
        );

        /* Controler la validite des registres sauves */
        assert(ebp_before == ebp_after);
        assert(esp_before == esp_after);
        assert(ebx == a1);
        assert(ecx == a2);
        assert(edx == a3);
        assert(edi == a4);
        assert(esi == a5);
}

int main(void *arg)
{
        (void)arg;
        it_ok = (unsigned*) shm_acquire("test9_shm");
        *it_ok = 0x0u;
        __test_valid_regs2(rand(), rand(), rand(), rand(), rand());
        shm_release("test9_shm");
        return 0;
}

