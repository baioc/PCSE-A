/*
 * userspace_tests.c
 *
 *  Created on: 26/02/2021
 *      Author: Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "userspace_tests.h"

#include "debug.h"
#include "stddef.h"
#include "string.h"
#include "stdlib.h"

#include "process.h"
#include "shared_memory.h"
#include "clock.h"

#include "test1.h"
#include "test4.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

// Imported from math.c in user/tests/lib
typedef unsigned long long uint_fast64_t;
typedef unsigned long      uint_fast32_t;

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static int test_0(void *arg);

static int test_1(void *arg);
static int dummy1(void *arg);
static int dummy2(void *arg);

static int test_2(void *arg);
static int procExit(void *args);
static int procKill(void *args);

static int test_3(void *arg);
static int prio4(void *arg);
static int prio5(void *arg);

static int  test_4(void *arg);
static int  busy1(void *arg);
static int  busy2(void *arg);
static void test_it(void);

static int test_5(void *arg);
static int no_run(void *arg);
static int waiter(void *arg);

static int                test_8(void *arg);
static int                suicide_launcher(void *arg);
static int                suicide(void *arg);
static unsigned long long div64(unsigned long long x, unsigned long long div,
                                unsigned long long *rem);

static int           test_9(void *arg);
static void          __test_valid_regs1(unsigned a1, unsigned a2, unsigned a3);
static int           proc_nothing(void *arg) __attribute__((unused));
static int           test_eax(void *arg);
static void          __test_valid_eax(unsigned a1);
static int           test_regs2(void *arg);
static void          __test_valid_regs2(unsigned a1, unsigned a2, unsigned a3,
                                        unsigned a4, unsigned a5);
static uint_fast32_t randBits(int _bits);
static unsigned long rand();
static unsigned long long mul64(unsigned long long x, unsigned long long y);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*
 * Test 9 variables
 */

// imported from math.c (user/tests/lib)
static const uint_fast64_t _multiplier = 0x5DEECE66DULL;
static const uint_fast64_t _addend = 0xB;
static const uint_fast64_t _mask = (1ULL << 48) - 1;
static uint_fast64_t       _seed = 1;
/*******************************************************************************
 * Public function
 ******************************************************************************/

void run_userspace_tests()
{
  int pid;

  pid = start(test_0, 0, 128, "test_0", 0);
  waitpid(pid, NULL);
  pid = start(test_1, 0, 128, "test_1", 0);
  waitpid(pid, NULL);
  pid = start(test_2, 0, 128, "test_2", 0);
  waitpid(pid, NULL);
  pid = start(test_3, 0, 128, "test_3", 0);
  waitpid(pid, NULL);
  pid = start(test_4, 0, 128, "test_4", 0);
  waitpid(pid, NULL);
  pid = start(test_5, 0, 128, "test_5", 0);
  waitpid(pid, NULL);
  pid = start(test_8, 0, 128, "test_8", 0);
  waitpid(pid, NULL);
  pid = start(test_9, 0, 128, "test_9", 0);
  waitpid(pid, NULL);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*-----------------*
 *      Test 0
 *-----------------*/
static int test_0(void *arg)
{
  (void)arg;
  register unsigned reg1 = 1u;
  register unsigned reg2 = 0xFFFFFFFFu;
  register unsigned reg3 = 0xBADB00B5u;
  register unsigned reg4 = 0xDEADBEEFu;

  printf("I'm a simple process running ...");

  unsigned i;
  for (i = 0; i < 10000000; i++) {
    if (reg1 != 1u || reg2 != 0xFFFFFFFFu || reg3 != 0xBADB00B5u ||
        reg4 != 0xDEADBEEFu)
    {
      printf(" and I feel bad. Bybye ...\n");
      assert(0);
    }
  }

  printf(" and I'm healthy. Leaving.\n");

  return 0;
}

/*-----------------*
 *      Test 1
 *-----------------*/
static int test_1(void *arg)
{
  int pid1;
  int r;
  int rval;

  (void)arg;

  pid1 = start(dummy1, 4000, 192, "dummy1", (void *)DUMMY_VAL);
  assert(pid1 > 0);
  printf(" 2");
  r = waitpid(pid1, &rval);
  assert(r == pid1);
  assert(rval == 3);
  printf(" 3");
  pid1 = start(dummy2, 4000, 100, "dummy2", (void *)(DUMMY_VAL + 1));
  assert(pid1 > 0);
  printf(" 4");
  r = waitpid(pid1, &rval);
  assert(r == pid1);
  assert(rval == 4);
  printf(" 6.\n");
  return 0;
}

static int dummy1(void *arg)
{
  printf("1");
  assert((int)arg == DUMMY_VAL);
  return 3;
}

static int dummy2(void *arg)
{
  printf(" 5");
  assert((int)arg == DUMMY_VAL + 1);
  return 4;
}

/*-----------------*
 *      Test 2
 *-----------------*/
static int test_2(void *arg)
{
  int rval;
  int r;
  int pid1;
  int val = 45;

  (void)arg;

  printf("1");
  pid1 = start(procKill, 4000, 100, "procKill", (void *)val);
  assert(pid1 > 0);
  printf(" 2");
  r = kill(pid1);
  assert(r == 0);
  printf(" 3");
  r = waitpid(pid1, &rval);
  assert(rval == 0);
  assert(r == pid1);
  printf(" 4");
  pid1 = start(procExit, 4000, 192, "procExit", (void *)val);
  assert(pid1 > 0);
  printf(" 6");
  r = waitpid(pid1, &rval);
  assert(rval == val);
  assert(r == pid1);
  assert(waitpid(getpid(), &rval) < 0);
  printf(" 7.\n");

  return 0;
}

static int procExit(void *args)
{
  printf(" 5");
  exit((int)args);
  assert(0);
  return 0;
}

static int procKill(void *args)
{
  printf(" X");
  return (int)args;
}

/*-----------------*
 *      Test 3
 *-----------------*/
static int test_3(void *arg)
{
  int pid1;
  int p = 192;
  int r;

  (void)arg;

  assert(getprio(getpid()) == 128);
  pid1 = start(prio4, 4000, p, "prio4", (void *)p);
  assert(pid1 > 0);
  printf(" 2");
  r = chprio(getpid(), 32);
  assert(r == 128);
  printf(" 4");
  r = chprio(getpid(), 128);
  assert(r == 32);
  printf(" 5");
  assert(waitpid(pid1, 0) == pid1);
  printf(" 6");

  assert(getprio(getpid()) == 128);
  pid1 = start(prio5, 4000, p, "prio5", (void *)p);
  assert(pid1 > 0);
  printf(" 8");
  r = kill(pid1);
  assert(r == 0);
  assert(waitpid(pid1, 0) == pid1);
  printf(" 9");
  r = chprio(getpid(), 32);
  assert(r == 128);
  printf(" 10");
  r = chprio(getpid(), 128);
  assert(r == 32);
  printf(" 11.\n");

  return 0;
}

static int prio4(void *arg)
{
  /* arg = priority of this proc. */
  int r;

  assert(getprio(getpid()) == (int)arg);
  printf("1");
  r = chprio(getpid(), 64);
  assert(r == (int)arg);
  printf(" 3");
  return 0;
}

static int prio5(void *arg)
{
  /* Arg = priority of this proc. */
  int r;

  assert(getprio(getpid()) == (int)arg);
  printf(" 7");
  r = chprio(getpid(), 64);
  assert(r == (int)arg);
  printf("error: I should have been killed\n");
  assert(0);
  return 0;
}

/*-----------------*
 *      Test 4
 *-----------------*/
static int test_4(void *args)
{
  int pid1, pid2;
  int r;
  int arg = 0;

  (void)args;

  assert(getprio(getpid()) == 128);
  pid1 = start(busy1, 4000, 64, "busy1", (void *)arg);
  assert(pid1 > 0);
  pid2 = start(busy2, 4000, 64, "busy2", (void *)pid1);
  assert(pid2 > 0);
  printf("1 -");
  r = chprio(getpid(), 32);
  assert(r == 128);
  printf(" - 2");
  r = kill(pid1);
  assert(r == 0);
  assert(waitpid(pid1, 0) == pid1);
  r = kill(pid2);
  assert(r < 0); /* kill d'un processus zombie */
  assert(waitpid(pid2, 0) == pid2);
  printf(" 3");
  r = chprio(getpid(), 128);
  assert(r == 32);
  printf(" 4.\n");

  return 0;
}

static int busy1(void *arg)
{
  (void)arg;
  while (1) {
    int i, j;

    printf(" A");
    for (i = 0; i < loop_count1; i++) {
      test_it();
      for (j = 0; j < loop_count0; j++)
	;
    }
  }
  return 0;
}

/* assume the process to suspend has a priority == 64 */
int busy2(void *arg)
{
  int i;

  for (i = 0; i < 3; i++) {
    int k, j;

    printf(" B");
    for (k = 0; k < loop_count1; k++) {
      test_it();
      for (j = 0; j < loop_count0; j++)
	;
    }
  }
  i = chprio((int)arg, 16);
  assert(i == 64);
  return 0;
}

static void test_it(void)
{
#ifdef microblaze
  int status, mstatus;
  __asm__ volatile(
      "mfs %0,rmsr; ori %1,%0,2; mts rmsr,%1; nop; nop; mts rmsr,%0"
      : "=r"(status), "=r"(mstatus));
#else
  __asm__ volatile("pushfl; testl $0x200,(%%esp); jnz 0f; sti; nop; cli; 0: "
                   "addl $4,%%esp\n" ::
                       : "memory");
#endif
}

/*-----------------*
 *      Test 5
 *-----------------*/
static int test_5(void *arg)
{
  int pid1, pid2;
  int r;

  (void)arg;

  // Le processus 0 et la priorite 0 sont des parametres invalides
  assert(kill(0) < 0);
  assert(chprio(getpid(), 0) < 0);
  assert(getprio(getpid()) == 128);
  pid1 = start(no_run, 4000, 64, "no_run", 0);
  assert(pid1 > 0);
  assert(kill(pid1) == 0);
  assert(kill(pid1) < 0);        // pas de kill de zombie
  assert(chprio(pid1, 128) < 0); // changer la priorite d'un zombie
  assert(chprio(pid1, 64) < 0);  // changer la priorite d'un zombie
  assert(waitpid(pid1, 0) == pid1);
  assert(waitpid(pid1, 0) < 0);
  pid1 = start(no_run, 4000, 64, "no_run", 0);
  assert(pid1 > 0);
  pid2 = start(waiter, 4000, 65, "waiter", (void *)pid1);
  assert(pid2 > 0);
  assert(waitpid(pid2, &r) == pid2);
  assert(r == 1);
  assert(waitpid(pid1, &r) == pid1);
  assert(r == 0);
  printf("ok.\n");

  return 0;
}

static int no_run(void *arg)
{
  (void)arg;
  assert(0);
  return 1;
}

static int waiter(void *arg)
{
  int pid = (int)arg;
  assert(kill(pid) == 0);
  assert(waitpid(pid, 0) < 0);
  return 1;
}

/*-----------------*
 *      Test 6
 *-----------------*/
// TODO: Find a way to add test_6

/*-----------------*
 *      Test 7
 *-----------------*/
// TODO: Add test_7 when shared memory is available (shm_{create/delete} needed)

/*-----------------*
 *      Test 8
 *-----------------*/
static int test_8(void *arg)
{
  unsigned long long tsc1;
  unsigned long long tsc2;
  int                i, r, pid, count;

  (void)arg;
  assert(getprio(getpid()) == 128);

  /* Le petit-fils va passer zombie avant le fils mais ne pas
     etre attendu par waitpid. Un nettoyage automatique doit etre
     fait. */
  pid = start(suicide_launcher, 4000, 129, "suicide_launcher", 0);
  assert(pid > 0);
  assert(waitpid(pid, &r) == pid);
  assert(chprio(r, 192) < 0);

  count = 0;
  __asm__ __volatile__("rdtsc" : "=A"(tsc1));
  do {
    for (i = 0; i < 10; i++) {
      pid = start(suicide_launcher, 4000, 200, "suicide_launcher", 0);
      assert(pid > 0);
      assert(waitpid(pid, 0) == pid);
    }
    test_it();
    count += i;
    __asm__ __volatile__("rdtsc" : "=A"(tsc2));
  } while ((tsc2 - tsc1) < 1000000000);
  printf("%lu cycles/process.\n",
         (unsigned long)div64(tsc2 - tsc1, 2 * (unsigned)count, 0));
  return 0;
}

static int suicide_launcher(void *arg)
{
  int pid1;
  (void)arg;
  pid1 = start(suicide, 4000, 192, "suicide", 0);
  assert(pid1 > 0);
  return pid1;
}

static int suicide(void *arg)
{
  (void)arg;
  kill(getpid());
  assert(0);
  return 0;
}

static unsigned long long div64(unsigned long long x, unsigned long long div,
                                unsigned long long *rem)
{
  unsigned long long mul = 1;
  unsigned long long q;

  if ((div > x) || !div) {
    if (rem) *rem = x;
    return 0;
  }

  while (!((div >> 32) & 0x80000000ULL)) {
    unsigned long long newd = div + div;
    if (newd > x) break;
    div = newd;
    mul += mul;
  }

  q = mul;
  x -= div;
  while (1) {
    mul /= 2;
    div /= 2;
    if (!mul) {
      if (rem) *rem = x;
      return q;
    }
    if (x < div) continue;
    q += mul;
    x -= div;
  }
}

/*-----------------*
 *      Test 9
 *-----------------*/
// test_9 main
static unsigned int ebp_before = 0x12345678u;
static unsigned int esp_before = 0x87654321u;
static unsigned int ebp_after = 0xDEADBEEFu;
static unsigned int esp_after = 0xDEADFACEu;
static unsigned int eax = 0xBADB00B5u;
static unsigned int ebx = 0xF0F0F0F0u;
static unsigned int edi = 0x0F0F0F0Fu;
static unsigned int esi = 0xABCDEFABu;

// test_regs2
static unsigned           ebx_regs2 = 1;
static unsigned           ecx_regs2 = 2;
static unsigned           edx_regs2 = 3;
static unsigned           edi_regs2 = 4;
static unsigned           esi_regs2 = 5;
static unsigned           ebp_before_regs2 = 0x12345678u;
static unsigned           esp_before_regs2 = 0x98765432u;
static unsigned           ebp_after_regs2 = 0xCAFE0101u;
static unsigned           esp_after_regs2 = 0x1010CAFEu;
static volatile unsigned *it_ok_regs2 = NULL;

// test_eax
static unsigned  eax_test_eax = 0xBADB00B5;
static unsigned *it_ok_test_eax = NULL;

__asm__("       .data                   \n"
        "nothing:                       \n"
        "       .string \"nothing\"     \n"
        "       .previous               \n");

static void __test_valid_regs1(unsigned a1, unsigned a2, unsigned a3)
{
  __asm__ __volatile__(
      /* Assigner des valeurs connues aux registres */
      "movl   %2,     %%ebx           \n"
      "movl   %3,     %%edi           \n"
      "movl   %4,    %%esi           \n"
      "movl   %%ebp,  %0              \n"
      "movl   %%esp,  %1              \n"

      /* Registres de sortie */
      : "=m"(ebp_before), "=m"(esp_before)
      /* Registres en entrée */
      : "m"(a1), "m"(a2), "m"(a3)
      /* Registres utilisés par ce bloc ASM*/
      : "ebx", "edi", "esi", "memory");

  __asm__ __volatile__(

      /* Démarrer le processus "nothing" */
      "pushl  $0                      \n"
      "pushl  $nothing                \n"
      "pushl  $192                    \n"
      "pushl  $4000                   \n"
      "pushl  $proc_nothing           \n"
      "call   start                   \n"
      "addl   $20,    %%esp           \n"

      /* Registres de sortie */
      :
      /* Registres en entrée */
      :
      /* Registres utilisés par ce bloc ASM*/
      : "eax", "memory");

  __asm__ __volatile__(
      /* Get return value of start (pid of proc_nothing) */
      "movl   %%eax,  %0              \n"

      /* Registres de sortie */
      : "=m"(eax)
      /* Registres en entrée */
      :
      /* Registres utilisés par ce bloc ASM*/
      : "eax", "memory");

  __asm__ __volatile__(

      /* Sauver les registres */
      "movl %%ebx,    %0              \n"
      "movl %%edi,    %1              \n"
      "movl %%esi,    %2              \n"
      "movl %%ebp,    %3              \n"
      "movl %%esp,    %4              \n"

      /* Registres de sortie */
      : "=m"(ebx), "=m"(edi), "=m"(esi), "=m"(ebp_after), "=m"(esp_after)
      /* Registres en entrée */
      :
      /* Registres utilisés par ce bloc ASM*/
      : "ebx", "edi", "esi", "memory");

  /* On attend le processus nothing dont le pid est dans eax */
  assert(waitpid((int)eax, NULL) == (int)eax);

  /* Vérifier les valeurs des registres après l'appel */
  assert(ebx == a1);
  assert(edi == a2);
  assert(esi == a3);
  assert(ebp_before == ebp_after);
  assert(esp_before == esp_after);
}

static int test_9(void *arg)
{
  int                i;
  int                pid;
  volatile unsigned *it_ok = NULL;

  (void)arg;
  it_ok = (unsigned *)shm_create("test9_shm");
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
    pid = start(test_regs2, 4000, 128, "test_regs2", 0);
    assert(pid > 0);
    while (*it_ok != 0)
      ;
    *it_ok = 1;
    assert(waitpid(pid, 0) == pid);
  }
  printf(" 3");

  /* Test de la cohérence de %eax */
  for (i = 0; i < 100; i++) {
    *it_ok = 1;
    pid = start(test_eax, 4000, 128, "test_eax", 0);
    assert(pid > 0);
    while (*it_ok != 0)
      ;
    *it_ok = 1;
    assert(waitpid(pid, 0) == pid);
  }
  printf(" 4.\n");

  shm_release("test9_shm");
  return 0;
}

static int proc_nothing(void *arg)
{
  (void)arg;
  return 0;
}

static void __test_valid_regs2(unsigned a1, unsigned a2, unsigned a3,
                               unsigned a4, unsigned a5)
{
  /* Initialise les registres avec des valeurs spéciales */
  __asm__ __volatile__("movl %2,       %%ebx    \n"
                       "movl %3,       %%ecx    \n"
                       "movl %4,       %%edx    \n"
                       "movl %5,       %%edi    \n"
                       "movl %6,       %%esi    \n"
                       /* Sauve le "stack pointer" et le "frame pointer" */
                       "movl %%ebp,     %0      \n"
                       "movl %%esp,     %1      \n"
                       : "=m"(ebp_before_regs2), "=m"(esp_before_regs2)
                       : "m"(a1), "m"(a2), "m"(a3), "m"(a4), "m"(a5)
                       : "%ebx", "%ecx", "%edx", "%edi", "%esi", "memory");

  /* Attendre au moins un scheduling (ie des interruptions) */
  while (*it_ok_regs2 == 0)
    ;

  /* Sauver les valeurs des registres */
  __asm__ __volatile__("movl %%ebp,    %0      \n"
                       "movl %%esp,    %1      \n"
                       "movl %%ebx,    %2      \n"
                       "movl %%ecx,    %3      \n"
                       "movl %%edx,    %4      \n"
                       "movl %%edi,    %5      \n"
                       "movl %%esi,    %6      \n"
                       : "=m"(ebp_after_regs2),
                         "=m"(esp_after_regs2),
                         "=m"(ebx_regs2),
                         "=m"(ecx_regs2),
                         "=m"(edx_regs2),
                         "=m"(edi_regs2),
                         "=m"(esi_regs2)
                       : /* No input registers */
                       : "memory");

  /* Controler la validite des registres sauves */
  assert(ebp_before_regs2 == ebp_after_regs2);
  assert(esp_before_regs2 == esp_after_regs2);
  assert(ebx_regs2 == a1);
  assert(ecx_regs2 == a2);
  assert(edx_regs2 == a3);
  assert(edi_regs2 == a4);
  assert(esi_regs2 == a5);
}

static int test_regs2(void *arg)
{
  (void)arg;
  it_ok_regs2 = (unsigned *)shm_acquire("test9_shm");
  *it_ok_regs2 = 0x0u;
  __test_valid_regs2(rand(), rand(), rand(), rand(), rand());
  shm_release("test9_shm");
  return 0;
}

static void __test_valid_eax(unsigned a1)
{
  __asm__ __volatile__(
      "   pushl %%eax                 \n" /* Sauver %eax */
      "   movl  %1,       %%eax       \n" /* Attendre un scheduling, ie que
                                           *it_ok vaille 1 */
      "0: testl $1,       %2          \n"
      "   jz    0b                    \n"
      "   movl  %%eax,    %0          \n" /* Récupérer la valeur d'%eax après
                                             interruption */
      "   popl  %%eax                 \n" /* Restaurer %eax */
      : "=m"(eax_test_eax)
      : "m"(a1), "m"(*it_ok_test_eax)
      : "%eax", "memory");
  /* %eax doit avoir conservé sa valeur avant interruption ! */
  assert(eax_test_eax == a1);
}

static int test_eax(void *arg)
{
  (void)arg;
  it_ok_test_eax = (unsigned *)shm_acquire("test9_shm");
  *it_ok_test_eax = 0x0u;
  __test_valid_eax(rand());
  shm_release("test9_shm");
  return 0;
}

// Imported from math.c in user/tests/lib
static uint_fast32_t randBits(int _bits)
{
  uint_fast32_t rbits;
  uint_fast64_t nextseed = (mul64(_seed, _multiplier) + _addend) & _mask;
  _seed = nextseed;
  rbits = nextseed >> 16;
  return rbits >> (32 - _bits);
}

// Imported from math.c in user/tests/lib
static unsigned long rand()
{
  return randBits(32);
}

// Imported from math.c in user/tests/lib
static unsigned long long mul64(unsigned long long x, unsigned long long y)
{
  unsigned long      a, b, c, d, e, f, g, h;
  unsigned long long res = 0;
  a = x & 0xffff;
  x >>= 16;
  b = x & 0xffff;
  x >>= 16;
  c = x & 0xffff;
  x >>= 16;
  d = x & 0xffff;
  e = y & 0xffff;
  y >>= 16;
  f = y & 0xffff;
  y >>= 16;
  g = y & 0xffff;
  y >>= 16;
  h = y & 0xffff;
  res = d * e;
  res += c * f;
  res += b * g;
  res += a * h;
  res <<= 16;
  res += c * e;
  res += b * f;
  res += a * g;
  res <<= 16;
  res += b * e;
  res += a * f;
  res <<= 16;
  res += a * e;
  return res;
}

/*-----------------*
 *      Test 10
 *-----------------*/
// TODO: Add test_10 when semaphore/message queue are available

/*-----------------*
 *      Test 11
 *-----------------*/
// TODO: Add test_11 when semaphore and shared memory are available

/*-----------------*
 *      Test 12
 *-----------------*/
// TODO: Add test_12 when semaphore/message queue are available

/*-----------------*
 *      Test 13
 *-----------------*/
// TODO: Add test_13 when semaphore/message queue and shared memory are
// available

/*-----------------*
 *      Test 14
 *-----------------*/
// TODO: Add test_14 when semaphore/message queue are available

/*-----------------*
 *      Test 15
 *-----------------*/
// TODO: Add test_15 when semaphore/message queue are available

/*-----------------*
 *      Test 16
 *-----------------*/
// TODO: Add test_16 when semaphore/message queue and shared memory are
// available

/*-----------------*
 *      Test 17
 *-----------------*/
// TODO: Add test_17 when semaphore/message queue and shared memory are
// available

/*-----------------*
 *      Test 18
 *-----------------*/
// "This test cannot work at kernel level"

/*-----------------*
 *      Test 19
 *-----------------*/
// TODO: Add test_19 when message queue is available

/*-----------------*
 *      Test 20
 *-----------------*/
// TODO: Add test_20 when semaphore and shared memory are available

/*-----------------*
 *      Test 21
 *-----------------*/
// TODO: Add test_21 when shared memory is available

/*-----------------*
 *      Test 22
 *-----------------*/
// TODO: Add test_22 when shared memory is available
