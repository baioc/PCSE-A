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
#include "process.h"
#include "debug.h"

#include "test1.h"
#include "test4.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

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

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

void run_userspace_tests()
{
  start(test_0, 0, 128, "test_0", 0);
  start(test_1, 0, 128, "test_1", 0);
  start(test_2, 0, 128, "test_2", 0);
  start(test_3, 0, 128, "test_3", 0);
  start(test_4, 0, 128, "test_4", 0);
  start(test_5, 0, 128, "test_5", 0);
  start(test_8, 0, 128, "test_8", 0);
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
// TODO: Add test_9 when shared memory is available (shm_{create/delete} needed)

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
