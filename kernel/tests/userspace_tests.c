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
#include "message-queue.h"
#include "shared_memory.h"
#include "clock.h"
#include "sem.h"
#include "test1.h"
#include "test4.h"
#include "test11.h"
#include "test13.h"
#include "test16.h"
#include "test21.h"

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

static int test_7(void *arg);
static int sleep_pr1(void *arg);
static int timer(void *arg);
static int timer1(void *arg);

static int                test_8(void *arg);
static int                suicide_launcher(void *arg);
static int                suicide(void *arg);
static unsigned long long div64(unsigned long long x, unsigned long long div,
                                unsigned long long *rem);

static void write_test_10(int fid, const char *buf, unsigned long len);
static void read_test_10(int fid, char *buf, unsigned long len);
static int test_10_msg(void *arg);

static int test_10_sem(void *arg);

static int test_11_sem(void *arg);
static int proc_mutex(void *arg);

static int test_12_msg(void *arg);
static int rdv_proc_12_msg(void *arg);

static int test_13_msg(void *arg);
static int p_receiver_13_msg(void *arg);
static int p_sender_13_msg(void *arg);

static int test_13_sem(void *arg);
static int proc13_1(void *arg);
static int proc13_2(void *arg);
static int proc13_3(void *arg);

 static int test_14_msg(void *arg);
 static int psender_14_1(void *arg);
 static int psender_14_2(void *arg);

 static int test_15_msg(void *arg);
 static int psmg_15_1(void *arg);
 static int psmg_15_2(void *arg);

static int test_12_sem(void *arg);
static int proc12_1(void *arg);
static int proc12_2(void * arg);
static int proc12_3(void *arg);

static int test_14_sem(void *arg);
static int proc14_1(void *arg);
static int proc14_2(void *arg);

static int test_15_sem(void *arg);

static int test_16_sem(void *arg);
static unsigned long test16_1(void);
static int proc16_1(void *arg);

static int test_17_sem(void *arg);
static int proc17_1(void *arg);
static int proc17_2(void *arg);

static int test_16_msg(void *arg);
static int proc_16_1_msg(void *arg);
static int proc_16_2_msg(void *arg);
static int proc_16_3_msg(void *arg);

static int proc_return(void *arg);
static int test_17_msg(void *arg);

static int test_20(void *arg);
static int launch_philo(void *arg);

static int test_21(void *arg);
static int shm_checker(void *arg);
/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Public function
 ******************************************************************************/

void run_userspace_tests()
{
  int pid;
if(true == false){
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

    pid = start(test_7, 0, 128, "test_7", 0);
    waitpid(pid, NULL);

  pid = start(test_8, 0, 128, "test_8", 0);
  waitpid(pid, NULL);
  pid = start(test_10_sem, 0, 128, "test_10_sem", 0);
  waitpid(pid,NULL);
  pid = start(test_10_msg, 0, 128, "test_10_msg", 0);
  waitpid(pid,NULL);
  pid = start(test_11_sem, 0, 128, "test_11_sem", 0);
  waitpid(pid,NULL);
  pid = start(test_12_msg, 0, 128, "test_12_msg", 0);
  waitpid(pid, NULL);
  pid = start(test_12_sem, 0, 128, "test_12_sem", 0);
  waitpid(pid, NULL);
  pid = start(test_13_msg, 0, 128, "test_13_msg", 0);
  waitpid(pid, NULL);
  pid = start(test_13_sem, 0, 128, "test_13_sem", 0);
  waitpid(pid, NULL);
  pid = start(test_14_sem, 0, 128, "test_14_sem", 0);
  waitpid(pid, NULL);
  pid = start(test_14_msg, 0, 128, "test_14_msg", 0);
  waitpid(pid, NULL);
  pid = start(test_15_msg, 0, 128, "test_15_msg", 0);
  waitpid(pid, NULL);
  pid = start(test_15_sem, 0, 128, "test_15_sem", 0);
  waitpid(pid, NULL);
}
  pid = start(test_16_sem, 0, 128, "test_16_sem", 0);
  waitpid(pid, NULL);
  pid = start(test_16_msg, 0, 128, "test_16_msg", 0);
  waitpid(pid, NULL);
  pid = start(test_17_msg, 0, 128, "test_17_msg", 0);
  waitpid(pid, NULL);
  pid = start(test_17_sem, 0, 128, "test_17_sem", 0);
  waitpid(pid, NULL);
  pid = start(test_20, 0, 128, "test_20", 0);
  waitpid(pid, NULL);
  pid = start(test_21, 0, 128, "test_21", 0);
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
// NOTE: test_6 assumes we make use of ssize parameter in function start.
// Irrelevant as we don't currently use that parameter

/*-----------------*
 *      Test 7
 *-----------------*/
static int test_7(void *arg)
{
  int                     pid1, pid2, r;
  unsigned long           c0, c, quartz, ticks, dur;
  volatile unsigned long *timer_test_7 = NULL;

  (void)arg;
  timer_test_7 = shm_create("test7_shm");
  assert(timer_test_7 != NULL);

  assert(getprio(getpid()) == 128);
  printf("1");
  pid1 = start(timer1, 4000, 129, "timer1", 0);
  assert(pid1 > 0);
  printf(" 3");
  assert(waitpid(-1, 0) == pid1);
  printf(" 8 : ");

  *timer_test_7 = 0;
  pid1 = start(timer, 4000, 127, "timer", 0);
  pid2 = start(timer, 4000, 127, "timer", 0);
  assert(pid1 > 0);
  assert(pid2 > 0);
  clock_settings(&quartz, &ticks);
  dur = 2 * quartz / ticks;
  test_it();
  c0 = current_clock();
  do {
    test_it();
    c = current_clock();
  } while (c == c0);
  wait_clock(c + dur);
  assert(kill(pid1) == 0);
  assert(waitpid(pid1, 0) == pid1);
  assert(kill(pid2) == 0);
  assert(waitpid(pid2, 0) == pid2);
  printf(
      "%lu changements de contexte sur %lu tops d'horloge", *timer_test_7, dur);
  pid1 = start(sleep_pr1, 4000, 192, "sleep_pr1", 0);
  assert(pid1 > 0);
  assert(kill(pid1) == 0);
  assert(waitpid(pid1, &r) == pid1);
  assert(r == 0);
  printf(".\n");
  shm_release("test7_shm");

  return 0;
}

static int sleep_pr1(void *arg)
{
  (void)arg;
  wait_clock(current_clock() + 2);
  printf(" not killed !!!");
  assert(0);
  return 1;
}

static int timer(void *arg)
{
  volatile unsigned long *timer = NULL;
  timer = shm_acquire("test7_shm");
  assert(timer != NULL);

  (void)arg;
  while (1) {
    unsigned long t = *timer + 1;
    *timer = t;
    while (*timer == t) test_it();
  }
  while (1)
    ;
  return 0;
}

static int timer1(void *arg)
{
  (void)arg;

  unsigned long quartz;
  unsigned long ticks;
  unsigned long dur;
  int           i;

  clock_settings(&quartz, &ticks);
  dur = (quartz + ticks) / ticks;
  printf(" 2");
  for (i = 4; i < 8; i++) {
    wait_clock(current_clock() + dur);
    printf(" %d", i);
  }
  return 0;
}

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
// NOTE: test_9 removed as it was not relevant on kernel side (no interrupts)

/*-----------------*
 *      Test 10
 *-----------------*/

int test_10_sem(void *arg)
{
        int sem1;
        (void)arg;
        sem1 = screate(2);
        assert(sem1 >= 0);
        assert(scount(sem1) == 2);
        assert(signal(sem1) == 0);
        assert(scount(sem1) == 3);
        assert(signaln(sem1, 2) == 0);
        assert(scount(sem1) == 5);
        assert(wait(sem1) == 0);
        assert(scount(sem1) == 4);
        assert(sreset(sem1, 7) == 0);
        assert(scount(sem1) == 7);
        assert(sdelete(sem1) == 0);
        printf("ok.\n");
        return 0;
}

 static void write_test_10(int fid, const char *buf, unsigned long len)
 {
         unsigned long i;
         for (i=0; i<len; i++) {
                 assert(psend(fid, buf[i]) == 0);
         }
 }

 static void read_test_10(int fid, char *buf, unsigned long len)
 {
         unsigned long i;
         for (i=0; i<len; i++) {
                 int msg;
                 assert(preceive(fid, &msg) == 0);
                 buf[i] = (char)msg;
         }
 }
 static int test_10_msg(void *arg)
 {
         int fid;
         const char *str = "abcde";
         unsigned long len = strlen(str);
         char buf[10];

         (void)arg;

         printf("1");
         assert((fid = pcreate(5)) >= 0);
         write_test_10(fid, str, len);
         printf(" 2");
         read_test_10(fid, buf, len);
         buf[len] = 0;
         assert(strcmp(str, buf) == 0);
         assert(pdelete(fid) == 0);
         printf(" 3.\n");
         return 0;
 }

/*-----------------*
 *      Test 11
 *-----------------*/

 void xwait(union sem *s)
 {
         assert(wait(s->sem) == 0);
 }

 void xsignal(union sem *s)
 {
         assert(signal(s->sem) == 0);
 }

 void xscreate(union sem *s)
 {
         assert((s->sem = screate(0)) >= 0);
 }

 void xsdelete(union sem *s)
 {
         assert(sdelete(s->sem) == 0);
 }

 static int test_11_sem(void *arg)
 {
         struct test11_shared *shared = NULL;
         int pid1, pid2, pid3, pid4;

         (void)arg;
         shared = (struct test11_shared*) shm_create("test11_shm");
         assert(shared != NULL);
         assert(getprio(getpid()) == 128);
         xscreate(&shared->sem);
         shared->in_mutex = 0;
         printf("1");

         pid1 = start(proc_mutex, 4000, 130, "proc_mutex", 0);
         pid2 = start(proc_mutex, 4000, 132, "proc_mutex", 0);
         pid3 = start(proc_mutex, 4000, 131, "proc_mutex", 0);
         pid4 = start(proc_mutex, 4000, 129, "proc_mutex", 0);
         assert(pid1 > 0);
         assert(pid2 > 0);
         assert(pid3 > 0);
         assert(pid4 > 0);
         assert(chprio(getpid(), 160) == 128);
         printf(" 6");
         xsignal(&shared->sem);
         assert(waitpid(-1, 0) == pid2);
         assert(waitpid(-1, 0) == pid3);
         assert(waitpid(-1, 0) == pid1);
         assert(waitpid(-1, 0) == pid4);
         assert(waitpid(-1, 0) < 0);
         assert(chprio(getpid(), 128) == 160);
         xsdelete(&shared->sem);
         printf(" 11.\n");

         return 0;
 }

 static int proc_mutex(void *arg)
 {
         struct test11_shared *shared = NULL;
         int p = -1;
         int msg;

         (void)arg;
         shared = (struct test11_shared*) shm_acquire("test11_shm");
         assert(shared != NULL);
         p = getprio(getpid());
         assert(p > 0);

         switch (p) {
                 case 130:
                         msg = 2;
                         break;
                 case 132:
                         msg = 3;
                         break;
                 case 131:
                         msg = 4;
                         break;
                 case 129:
                         msg = 5;
                         break;
                 default:
                         msg = 15;
         }
         printf(" %d", msg);
         xwait(&shared->sem);
         printf(" %d", 139 - p);
         assert(!(shared->in_mutex++));
         chprio(getpid(), 16);
         chprio(getpid(), p);
         shared->in_mutex--;
         xsignal(&shared->sem);
         return 0;
 }

/*-----------------*
 *      Test 12
 *-----------------*/

 static int test_12_sem(void *arg)
 {
         int sem;
         int pid1, pid2, pid3;
         int ret;

         (void)arg;

         assert(getprio(getpid()) == 128);
         assert((sem = screate(1)) >= 0);
         pid1 = start(proc12_1, 4000, 129, "proc12_1", (void *)sem);
         assert(pid1 > 0);
         printf(" 2");
         pid2 = start(proc12_2, 4000, 127, "proc12_2", (void *)sem);
         assert(pid2 > 0);
         pid3 = start(proc12_3, 4000, 130, "proc12_3", (void *)sem);
         assert(pid3 > 0);
         printf(" 4");
         assert(chprio(getpid(), 126) == 128);
         printf(" 6");
         assert(chprio(getpid(), 128) == 126);
         assert(signaln(sem, 2) == 0);
         assert(signaln(sem, 1) == 0);
         assert(signaln(sem, 4) == 0);
         assert(waitpid(pid1, &ret) == pid1);
         assert(ret == 1);
         assert(waitpid(-1, &ret) == pid3);
         assert(ret == 0);
         assert(scount(sem) == 1);
         assert(sdelete(sem) == 0);
         printf(" 12");
         assert(waitpid(-1, &ret) == pid2);
         assert(ret == 2);
         printf(" 14.\n");
         return 0;
 }

 static int proc12_1(void *arg)
 {
         int sem = (int)arg;
         assert(try_wait(sem) == 0);
         assert(try_wait(sem) == -3);
         printf("1");
         assert(wait(sem) == 0);
         printf(" 8");
         assert(wait(sem) == 0);
         printf(" 11");
         exit(1);
         return 0;
 }

 static int proc12_2(void * arg)
 {
         int sem = (int)arg;
         printf(" 5");
         assert(wait(sem) == 0);
         printf(" 13");
         return 2;
 }

 static int proc12_3(void *arg)
 {
         int sem = (int)arg;
         printf(" 3");
         assert(wait(sem) == 0);
         printf(" 7");
         assert(wait(sem) == 0);
         printf(" 9");
         assert(wait(sem) == 0);
         printf(" 10");
         kill(getpid());
         assert(!"Should not arrive here !");
         while(1);
         return 0;
 }

 int rdv_proc_12_msg(void *arg)
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

 static int test_12_msg(void *arg){
   int fid;
   int pid;
   int msg;
   int count;

   (void)arg;

   assert(getprio(getpid()) == 128);
   assert((fid = pcreate(1)) >= 0);
   printf("1");
   pid = start(rdv_proc_12_msg, 4000, 130, "rdv_proc_12_msg", (void *)fid);
   assert(pid > 0);
   printf(" 4");
   assert((pcount(fid, &count) == 0) && (count == 2));
   assert(preceive(fid, &msg) == 0); /* Retire du tampon et debloque un emetteur. */
   assert(msg == 3);
   printf(" 7");
   assert((pcount(fid, &count) == 0) && (count == -1));
   assert(psend(fid, 5) == 0); /* Pose dans le tampon. */
   printf(" 9");
   assert(psend(fid, 6) == 0); /* Pose dans le tampon. */
   assert(preceive(fid, &msg) == 0); /* Retire du tampon. */
   assert(msg == 6);
   assert(pdelete(fid) == 0);
   assert(psend(fid, 2) < 0);
   assert(preceive(fid, &msg) < 0);
   assert(waitpid(-1, 0) == pid);
   printf(" 10.\n");
   return 0;
 }

/*-----------------*
 *      Test 13
 *-----------------*/

 static int p_receiver_13_msg(void *arg)
 {
         struct psender *ps = NULL;
         int ps_index = (int)arg;
         int msg;
         unsigned i;
         unsigned n;

         ps = shm_acquire("test13_shm");
         assert(ps != NULL);
         n = strlen(ps[ps_index].data);

         for(i = 0; i < n; i++) {
                 assert(preceive(ps[ps_index].fid, &msg) == 0);
                 assert(msg == ps[ps_index].data[i]);
         }

         shm_release("test13_shm");
         return 0;
 }

 int p_sender_13_msg(void *arg)
 {
         struct psender *ps = NULL;
         int ps_index = (int)arg;
         unsigned i;
         unsigned n;

         ps = shm_acquire("test13_shm");
         assert(ps != NULL);
         n = strlen(ps[ps_index].data);

         for(i = 0; i < n; i++) {
                 assert(psend(ps[ps_index].fid, ps[ps_index].data[i]) == 0);
         }
         shm_release("test13_shm");
         return 0;
 }


 static int test_13_msg(void *arg)
 {
         struct psender *ps = NULL;
         int pid1, pid2, pid3;
         int fid = pcreate(3);
         int i, msg;

         (void)arg;
         ps = (struct psender*) shm_create("test13_shm");
         assert(ps != NULL);

         printf("1");
         assert(getprio(getpid()) == 128);
         assert(fid >= 0);
         ps[1].fid = ps[2].fid = ps[3].fid = fid;
         strncpy(ps[1].data, "abcdehm", 32);
         strncpy(ps[2].data, "il", 32);
         strncpy(ps[3].data, "fgjk", 32);
         pid1 = start(p_sender_13_msg, 4000, 131, "p_sender_13_msg_1", (void*)1);
         pid2 = start(p_sender_13_msg, 4000, 130, "p_sender_13_msg_2", (void*)2);
         pid3 = start(p_sender_13_msg, 4000, 129, "p_sender_13_msg_3", (void*)3);
         for (i=0; i<2; i++) {
                 assert(preceive(fid, &msg) == 0);
                 assert(msg == 'a' + i);
         }
         chprio(pid1, 129);
         chprio(pid3, 131);
         for (i=0; i<2; i++) {
                 assert(preceive(fid, &msg) == 0);
                 assert(msg == 'c' + i);
         }
         chprio(pid1, 127);
         chprio(pid2, 126);
         chprio(pid3, 125);
         for (i=0; i<6; i++) {
                 assert(preceive(fid, &msg) == 0);
                 assert(msg == 'e' + i);
         }
         chprio(pid1, 125);
         chprio(pid3, 127);
         for (i=0; i<3; i++) {
                 assert(preceive(fid, &msg) == 0);
                 assert(msg == 'k' + i);
         }
         assert(waitpid(pid3, 0) == pid3); //XXX assert(waitpid(-1, 0) == pid3); ???
         assert(waitpid(-1, 0) == pid2);
         assert(waitpid(-1, 0) == pid1);
         printf(" 2");

         strncpy(ps[1].data, "abej", 32);
         strncpy(ps[2].data, "fi", 32);
         strncpy(ps[3].data, "cdgh", 32);
         pid1 = start(p_receiver_13_msg, 4000, 131, "p_receiver_13_msg_1", (void*)1);
         pid2 = start(p_receiver_13_msg, 4000, 130, "p_receiver_13_msg_2", (void*)2);
         pid3 = start(p_receiver_13_msg, 4000, 129, "p_receiver_13_msg_3", (void*)3);
         for (i='a'; i<='b'; i++) {
                 assert(psend(fid, i) == 0);
         }
         chprio(pid1, 129);
         chprio(pid3, 131);
         for (i='c'; i<='d'; i++) {
                 assert(psend(fid, i) == 0);
         }
         chprio(pid1, 127);
         chprio(pid2, 126);
         chprio(pid3, 125);
         for (i='e'; i<='j'; i++) {
                 assert(psend(fid, i) == 0);
         }
         chprio(pid1, 125);
         chprio(pid3, 127);
         assert(waitpid(-1, 0) == pid3);
         assert(waitpid(-1, 0) == pid2);
         assert(waitpid(-1, 0) == pid1);
         assert(pdelete(fid) == 0);
         printf(" 3.\n");
         shm_release("test13_shm");
         return 0;
 }

 static int test_13_sem(void *arg)
 {
         int sem;
         int pid1, pid2, pid3;
         int ret;

         (void)arg;

         assert(getprio(getpid()) == 128);
         assert((sem = screate(1)) >= 0);
         pid1 = start(proc13_1, 4000, 129, "proc13_1", (void *)sem);
         assert(pid1 > 0);
         printf(" 2");
         pid2 = start(proc13_2, 4000, 127, "proc13_2", (void *)sem);
         assert(pid2 > 0);
         pid3 = start(proc13_3, 4000, 130, "proc13_3", (void *)sem);
         assert(pid3 > 0);
         printf(" 4");
         assert(chprio(getpid(), 126) == 128);
         printf(" 6");
         assert(chprio(getpid(), 128) == 126);
         assert(sreset(sem, 1) == 0);
         printf(" 10");
         assert(chprio(getpid(), 126) == 128);
         assert(chprio(getpid(), 128) == 126);
         assert(sdelete(sem) == 0);
         printf(" 14");
         assert(waitpid(pid1, &ret) == pid1);
         assert(ret == 1);
         assert(waitpid(-1, &ret) == pid3);
         assert(ret == 3);
         assert(waitpid(-1, &ret) == pid2);
         assert(ret == 2);
         assert(signal(sem) == -1);
         assert(scount(sem) == -1);
         assert(sdelete(sem) == -1);
         printf(" 16.\n");
         return 0;
 }

 static int proc13_1(void *arg)
 {
         int sem = (int)arg;
         assert(try_wait(sem) == 0);
         assert(try_wait(sem) == -3);
         printf("1");
         assert(wait(sem) == -4);
         printf(" 9");
         assert(wait(sem) == -3);
         printf(" 13");
         assert(wait(sem) == -1);

         exit(1);
 }

 static int proc13_2(void *arg)
 {
         int sem = (int)arg;
         printf(" 5");
         assert(wait(sem) == -4);
         printf(" 11");
         assert(wait(sem) == -3);
         printf(" 15");
         assert(wait(sem) == -1);

         return 2;
 }

 static int proc13_3(void *arg)
 {
         int sem = (int)arg;
         printf(" 3");
         assert(wait(sem) == -4);
         printf(" 7");
         assert(wait(sem) == 0);
         printf(" 8");
         assert(wait(sem) == -3);
         printf(" 12");
         assert(wait(sem) == -1);
         exit(3);
         assert(!"Should not arrive here !");
         while(1);
 }

/*-----------------*
 *      Test 14
 *-----------------*/
 static int test_14_sem(void *arg)
 {
         int sem;
         int pid1, pid2;

         (void)arg;

         assert(getprio(getpid()) == 128);
         assert((sem = screate(0)) >= 0);
         pid1 = start(proc14_1, 4000, 129, "proc14_1", (void *)sem);
         assert(pid1 > 0);
         printf(" 2");
         pid2 = start(proc14_2, 4000, 130, "proc14_2", (void *)sem);
         assert(pid2 > 0);
         printf(" 4");
         assert(chprio(pid1, 131) == 129);
         assert(signal(sem) == 0);
         printf(" 6");
         assert(chprio(pid1, 127) == 131);
         assert(signal(sem) == 0);
         printf(" 8");
         assert(signaln(sem, 2) == 0);
         printf(" 10");
         assert(waitpid(pid1, 0) == pid1);
         assert(scount(sem) == 0xffff);
         assert(kill(pid2) == 0);
         assert(getprio(pid2) < 0);
         assert(chprio(pid2, 129) < 0);
         assert(scount(sem) == 0);
         assert(signal(sem) == 0);
         assert(scount(sem) == 1);
         assert(sdelete(sem) == 0);
         assert(waitpid(-1, 0) == pid2);
         printf(" 12.\n");
         return 0;
 }

 static int proc14_1(void *arg)
 {
         int sem = (int)arg;
         printf("1");
         assert(wait(sem) == 0);
         printf(" 5");
         assert(wait(sem) == 0);
         printf(" 11");
         exit(1);
 }

 static int proc14_2(void *arg)
 {
         int sem = (int)arg;
         printf(" 3");
         assert(wait(sem) == 0);
         printf(" 7");
         assert(wait(sem) == 0);
         printf(" 9");
         assert(wait(sem) == 0);
         printf(" X");
         return 2;
 }


  static int psender_14_1(void *arg)
  {
          int fid1 = (int)arg;
          int fid2;
          int msg;

          printf(" 2 t14");
          assert(preceive(fid1, &fid2) == 0);
          assert(psend(fid1, fid2) == 0);
          fid2 -= 42;
          assert(psend(fid1, 1) == 0);
          assert(psend(fid1, 2) == 0);
          assert(psend(fid1, 3) == 0);

          assert(psend(fid1, 4) == 0);

          assert(psend(fid1, 5) < 0);

          printf(" 6 t14");
          assert(psend(fid1, 12) < 0);
          printf(" 9 t14");
          assert(psend(fid1, 14) < 0);
          assert(preceive(fid2, &msg) < 0);
          printf(" 12 t14");
          assert(preceive(fid2, &msg) < 0);
          assert(preceive(fid2, &msg) < 0);
          return 0;
  }

  static int psender_14_2(void *arg)
  {
          int fid1 = (int)arg;
          int fid2;
          int msg;

          printf(" 3 t14");
          assert(preceive(fid1, &fid2) == 0);

          fid2 -= 42;
          assert(psend(fid1, 6) < 0);

          printf(" 5 t14");
          assert(psend(fid1, 7) == 0);
          assert(psend(fid1, 8) == 0);
          assert(psend(fid1, 9) == 0);
          assert(psend(fid1, 10) == 0);
          assert(psend(fid1, 11) < 0);
          printf(" 8 t14");
          assert(psend(fid1, 13) < 0);
          assert((preceive(fid2, &msg) == 0) && (msg == 15));
          assert(preceive(fid2, &msg) < 0);
          printf(" 11 t14");
          assert(preceive(fid2, &msg) < 0);
          assert(preceive(fid2, &msg) < 0);
          return 0;
  }

  static int test_14_msg(void *arg)
  {
          int pid1, pid2;
          int fid1 = pcreate(3);
          int fid2 = pcreate(3);
          int msg;

          (void)arg;

          /* Bravo si vous n'etes pas tombe dans le piege. */
          assert(pcreate(1073741827) < 0);

          printf(" 1 t14");
          assert(getprio(getpid()) == 128);
          assert(fid1 >= 0);
          assert(psend(fid1, fid2 + 42) == 0);
          pid1 = start(psender_14_1, 4000, 131, "psender_14_1", (void *)fid1);

          pid2 = start(psender_14_2, 4000, 130, "psender_14_2", (void *)fid1);

          assert((preceive(fid1, &msg) == 0) && (msg == 1));

          assert(chprio(pid2, 132) == 130);
          printf(" 4 t14");

          assert(preset(fid1) == 0);

          assert((preceive(fid1, &msg) == 0) && (msg == 7));
          printf(" 7 t14");
          assert(pdelete(fid1) == 0);

          printf(" 10 t14");
          assert(psend(fid2, 15) == 0);
          assert(preset(fid2) == 0);
          printf(" 13 t14");
          assert(pdelete(fid2) == 0);
          assert(pdelete(fid2) < 0);
          assert(waitpid(pid2, 0) == pid2); //XXX assert(waitpid(-1, 0) == pid2); ???
          assert(waitpid(-1, 0) == pid1);
          printf(".\n");
          return 0;
        }

/*-----------------*
 *      Test 15
 *-----------------*/
 static int test_15_sem(void *arg)
 {
         int sem, i;

         (void)arg;

         assert(screate(-2) == -1);
         assert((sem = screate(2)) >= 0);
         assert(signaln(sem, -4) < 0);
         assert(sreset(sem, -3) == -1);
         assert(scount(sem) == 2);
         assert(signaln(sem, 32760) == 0);
         assert(signaln(sem, 6) == -2);
         assert(scount(sem) == 32762);
         assert(wait(sem) == 0);
         assert(scount(sem) == 32761);
         assert(signaln(sem, 30000) == -2);
         assert(scount(sem) == 32761);
         assert(wait(sem) == 0);
         assert(scount(sem) == 32760);
         assert(signaln(sem, -2) < 0);
         assert(scount(sem) == 32760);
         assert(wait(sem) == 0);
         assert(scount(sem) == 32759);
         assert(signaln(sem, 8) == 0);
         assert(scount(sem) == 32767);
         assert(signaln(sem, 1) == -2);
         assert(scount(sem) == 32767);
         assert(signal(sem) == -2);
         assert(scount(sem) == 32767);
         for (i=0; i<32767; i++) {
                 assert(wait(sem) == 0);
         }
         assert(try_wait(sem) == -3);
         assert(scount(sem) == 0);
         assert(sdelete(sem) == 0);
         printf("ok.\n");
         return 0;
 }

 static int psmg_15_1(void *arg)
 {
         int fid1 = (int)arg;

         printf(" 2");
         assert(psend(fid1, 1) == 0);
         assert(psend(fid1, 2) == 0);
         assert(psend(fid1, 3) == 0);
         assert(psend(fid1, 4) == 0);
         assert(psend(fid1, 5) == 457);
         return 1;
 }

 static int psmg_15_2(void *arg)
 {
         int fid1 = (int)arg;

         printf(" 3");
         assert(psend(fid1, 6) == 0);
         assert(psend(fid1, 7) == 457);
         return 1;
 }

static int test_15_msg(void *arg)
{
        int pid1, pid2, fid1;
        int msg;
        int count = 1;
        int r = 1;

        (void)arg;

        assert((fid1 = pcreate(3)) >= 0);
        printf("1");
        assert(getprio(getpid()) == 128);
        pid1 = start(psmg_15_1, 4000, 131, "psmg_15_1", (void *)fid1);
        assert(pid1 > 0);
        pid2 = start(psmg_15_2, 4000, 130, "pmsg_15_2", (void *)fid1);
        assert(pid2 > 0);

        assert((preceive(fid1, &msg) == 0) && (msg == 1));
        assert(kill(pid1) == 0);
        assert(kill(pid1) < 0);
        assert((preceive(fid1, &msg) == 0) && (msg == 2));
        assert(kill(pid2) == 0);
        assert(kill(pid2) < 0);
        assert(preceive(fid1, &msg) == 0);
        assert(msg == 3);
        assert(preceive(fid1, &msg) == 0);
        assert(msg == 4);
        assert(preceive(fid1, &msg) == 0);
        assert(msg == 6);
        assert(pcount(fid1, &count) == 0);
        assert(count == 0);
        assert(waitpid(pid1, &r) == pid1);
        assert(r == 0);
        r = 1;
        assert(waitpid(-1, &r) == pid2);
        assert(r == 0);
        assert(pdelete(fid1) == 0);
        assert(pdelete(fid1) < 0);
        printf(" 4.\n");
        return 0;
}

/*-----------------*
 *      Test 16
 *-----------------*/

 #define NBSEMS 10000

 typedef unsigned long long uint_fast64_t;
 typedef unsigned long uint_fast32_t;

 static const uint_fast64_t _multiplier = 0x5DEECE66DULL;
 static const uint_fast64_t _addend = 0xB;
 static const uint_fast64_t _mask = (1ULL << 48) - 1;
 static uint_fast64_t _seed = 1;

 static unsigned long long mul64(unsigned long long x, unsigned long long y)
 {
         unsigned long a, b, c, d, e, f, g, h;
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

 static uint_fast32_t randBits(int _bits)
 {
         uint_fast32_t rbits;
         uint_fast64_t nextseed = (mul64(_seed, _multiplier) + _addend) & _mask;
         _seed = nextseed;
         rbits = nextseed >> 16;
         return rbits >> (32 - _bits);
 }

 short randShort()
 {
         return randBits(15);
 }

 void setSeed(uint_fast64_t _s)
 {
         _seed = _s;
 }

 static int test_16_sem(void *arg)
 {
         int pid;
         (void)arg;
         pid = start(proc16_1, 4000 + NBSEMS * 4, 128, "proc16_1", 0);
         assert(pid > 0);
         assert(waitpid(pid, 0) == pid);
         return 0;
 }

 static unsigned long test16_1(void)
 {
         unsigned long long tsc, tsc1, tsc2;
         unsigned long count = 0;

         __asm__ __volatile__("rdtsc":"=A"(tsc1));
         tsc2 = tsc1 + 1000000000;
         assert(tsc1 < tsc2);
         do {
                 unsigned i;
                 test_it();
                 for (i=0; i<100; i++) {
                         int sem1 = screate(2);
                         int sem2 = screate(1);
                         assert(sem1 >= 0);
                         assert(sem2 >= 0);
                         assert(sem1 != sem2);
                         assert(sdelete(sem1) == 0);
                         assert(sdelete(sem2) == 0);
                 }
                 __asm__ __volatile__("rdtsc":"=A"(tsc));
                 count += 2 * i;

         } while (tsc < tsc2);
         return (unsigned long)div64(tsc - tsc1, count, 0);
 }

 static int proc16_1(void *arg)
 {
         int sems[NBSEMS];
         int i;
         unsigned long c1, c2;
         unsigned long long seed;

         (void)arg;

         c1 = test16_1();
         printf("%lu ", c1);
         __asm__ __volatile__("rdtsc":"=A"(seed));
         setSeed(seed);
         for (i=0; i<NBSEMS; i++) {
                 int sem = screate(randShort());
                 if (sem < 0) assert(!"*** Increase the semaphore capacity of your system to NBSEMS to pass this test. ***");
                 sems[i] = sem;
         }
         if (screate(0) >= 0) assert(!"*** Decrease the semaphore capacity of your system to NBSEMS to pass this test. ***");
         assert(sdelete(sems[NBSEMS/3]) == 0);
         assert(sdelete(sems[(NBSEMS/3)*2]) == 0);
         for(int i = 0; i < NBSEMS; i++) printf("%d | %d , ",i,sems[i]);
         c2 = test16_1();
         printf("%lu ", c2);
         setSeed(seed);
         for (i=0; i<NBSEMS; i++) {
                 //printf("%d\n",i);
                 short randVal = randShort();
                 if ((i != (NBSEMS/3)) && (i != (2*(NBSEMS/3)))) {
                         assert(scount(sems[i]) == randVal);// Bloque à i = 8725
                         assert(sdelete(sems[i]) == 0);
                 }
         }
         if (c2 < 2 * c1)
                 printf("ok.\n");
         else
                 printf("Bad algorithm complexity in semaphore allocation.\n");
         return 0;
 }

static int proc_16_1_msg(void *arg)
{
        struct tst16 *p = NULL;
        int i, msg;

        (void)arg;
        p = shm_acquire("test16_shm");
        assert(p != NULL);

        for (i = 0; i <= p->count; i++) {
                assert(preceive(p->fid, &msg) == 0);
                assert(msg == i);
                test_it();
        }
        shm_release("test16_shm");
        return 0;
}

static int proc_16_2_msg(void *arg)
{
        struct tst16 *p = NULL;
        int i, msg;

        (void)arg;
        p = shm_acquire("test16_shm");
        assert(p != NULL);

        for (i = 0; i < p->count; i++) {
                assert(preceive(p->fid, &msg) == 0);
                test_it();
        }
        shm_release("test16_shm");
        return 0;
}

static int proc_16_3_msg(void *arg)
{
        struct tst16 *p = NULL;
        int i;

        (void)arg;
        p = shm_acquire("test16_shm");
        assert(p != NULL);

        for (i = 0; i < p->count; i++) {
                assert(psend(p->fid, i) == 0);
                test_it();
        }
        shm_release("test16_shm");
        return 0;
}

static int test_16_msg(void *arg)
{
        int i, count, fid, pid;
        struct tst16 *p = NULL;
        int pids[2 * NB_PROCS];

        (void)arg;
        p = (struct tst16*) shm_create("test16_shm");
        assert(p != NULL);

        assert(getprio(getpid()) == 128);
        for (count = 1; count <= 100; count++) {
                fid = pcreate(count);
                assert(fid >= 0);
                p->count = count;
                p->fid = fid;
                pid = start(proc_16_1_msg, 2000, 128, "proc16_1_msg", 0);
                assert(pid > 0);
                for (i=0; i<=count; i++) {
                        assert(psend(fid, i) == 0);
                        test_it();
                }
                assert(waitpid(pid, 0) == pid);
                assert(pdelete(fid) == 0);
        }

        p->count = 20000;
        fid = pcreate(50);
        assert(fid >= 0);
        p->fid = fid;
        for (i = 0; i< NB_PROCS; i++) {
                pid = start(proc_16_2_msg, 2000, 127, "proc16_2_msg", 0);
                assert(pid > 0);
                pids[i] = pid;
        }
        for (i=0; i < NB_PROCS; i++) {
                pid = start(proc_16_3_msg, 2000, 127, "proc16_3_msg", 0);
                assert(pid > 0);
                pids[NB_PROCS + i] = pid;
        }
        for (i=0; i < 2 * NB_PROCS; i++) {
                assert(waitpid(pids[i], 0) == pids[i]);
        }
        assert(pcount(fid, &count) == 0);
        assert(count == 0);
        assert(pdelete(fid) == 0);

        shm_release("test16_shm");
        printf("ok test 16.\n");
        return 0;
}
/*-----------------*
 *      Test 17
 *-----------------*/

 struct test17_buf_st {
     int mutex;
     int wsem;
     unsigned wpos;
     int rsem;
     unsigned rpos;
     char buf[100];
     int received[256];
 };

 // Increment a variable in a single atomic operation
 static __inline__ void atomic_incr(int *atomic)
 {
     __asm__ __volatile__("incl %0" : "+m" (*atomic) : : "cc");
 }

 static int test_17_sem(void *arg)
 {
         int pid[6];
         int i;
         struct test17_buf_st *st = NULL;
         int count = 0;

         (void)arg;
         st = (struct test17_buf_st*) shm_create("test17_shm");
         assert(st != NULL);

         assert(getprio(getpid()) == 128);
         st->mutex = screate(1);
         assert(st->mutex >= 0);
         st->wsem = screate(100);
         assert(st->wsem >= 0);
         st->wpos = 0;
         st->rsem = screate(0);
         assert(st->rsem >= 0);
         st->rpos = 0;
         for (i=0; i<256; i++) {
                 st->received[i] = 0;
         }
         for (i=0; i<3; i++) {
                 pid[i] = start(proc17_1, 4000, 129, "proc17_1", &st);
                 assert(pid[i] > 0);
         }
         for (i=3; i<6; i++) {
                 pid[i] = start(proc17_2, 4000, 129, "proc17_2", &st);
                 assert(pid[i] > 0);
         }
         for (i=0; i<3; i++) {
                 int ret;
                 assert(waitpid(pid[i], &ret) == pid[i]);
                 count += ret;
         }
         assert(scount(st->rsem) == 0xfffd);
         for (i=3; i<6; i++) {
                 int ret;
                 assert(kill(pid[i]) == 0);
                 assert(waitpid(pid[i], &ret) == pid[i]);
         }
         assert(scount(st->mutex) == 1);
         assert(scount(st->wsem) == 100);
         assert(scount(st->rsem) == 0);
         assert(sdelete(st->mutex) == 0);
         assert(sdelete(st->wsem) == 0);
         assert(sdelete(st->rsem) == 0);
         for (i=0; i<256; i++) {
                 int n = st->received[i];
                 if (n != count) {
                         printf("st->received[%d] == %d, count == %d\n", i, n, count);
                         assert(n == count);
                 }
         }
         printf("ok (%d chars sent).\n", count * 256);
         return 0;
 }

 static void buf_send(char x, struct test17_buf_st *st)
 {
     assert(wait(st->wsem) == 0);
     assert(wait(st->mutex) == 0);
     st->buf[(st->wpos++) % sizeof(st->buf)] = x;
     assert(signal(st->mutex) == 0);
     assert(signal(st->rsem) == 0);
 }

 static int proc17_1(void *arg)
 {
         struct test17_buf_st *st = NULL;
         unsigned long long tsc, tsc2;
         int count;

         (void)arg;

         st = (struct test17_buf_st*) shm_acquire("test17_shm");

         __asm__ __volatile__("rdtsc":"=A"(tsc));
         tsc2 = tsc + 1000000000;
         assert(tsc < tsc2);
         do {
                 int j;
                 for (j=0; j<256; j++) {
                         buf_send((char)j, st);
                 }
                 count++;
                 __asm__ __volatile__("rdtsc":"=A"(tsc));
         } while (tsc < tsc2);
         shm_release("test17_shm");
         return count;
 }

 static int buf_receive(struct test17_buf_st *st)
 {
     int x;
     assert(wait(st->rsem) == 0);
     assert(wait(st->mutex) == 0);
     x = 0xff & (int)(st->buf[(st->rpos++) % sizeof(st->buf)]);
     assert(signal(st->mutex) == 0);
     assert(signal(st->wsem) == 0);
     return x;
 }

 static int proc17_2(void *arg)
 {
         struct test17_buf_st *st = NULL;

         (void)arg;

         st = (struct test17_buf_st*) shm_acquire("test17_shm");
         assert(st != NULL);

         while(1) {
                 int x = buf_receive(st);
                 atomic_incr(&st->received[x]);
         }
         shm_release("test17_shm");
         return 0;
 }

 static int ids[1200];

 static const int heap_len = 64 << 20;

 static int proc_return(void *arg)
 {
         return (int)arg;
 }

 static int test_17_msg(void *arg)
 {
         int i, n, nx;
         int l = sizeof(ids) / sizeof(int);
         int count;
         int prio;

         (void)arg;

         n = 0;
         while (1) {
                 int fid = pcreate(1);
                 if (fid < 0) break;
                 ids[n++] = fid;
                 if (n == l) {
                         assert(!"Maximum number of queues too high !");
                 }
                 test_it();
         }
         for (i=0; i<n; i++) {
                 assert(pdelete(ids[i]) == 0);
                 test_it();
         }
         for (i=0; i<n; i++) {
                 int fid = pcreate(1);
                 assert(fid >= 0);
                 ids[i] = fid;
                 test_it();
         }
         assert(pcreate(1) < 0);
         for (i=0; i<n; i++) {
                 assert(pdelete(ids[i]) == 0);
                 test_it();
         }
         printf("%d", n);

         for (i=0; i<n; i++) {
                 int fid = pcreate(1);
                 assert(fid >= 0);
                 assert(psend(fid, i) == 0);
                 ids[i] = fid;
                 test_it();
         }
         assert(pcreate(1) < 0);
         for (i=0; i<n; i++) {
                 int msg;
                 assert(preceive(ids[i], &msg) == 0);
                 assert(msg == i);
                 assert(pdelete(ids[i]) == 0);
                 test_it();
         }

         count = heap_len / (int)sizeof(int);
         count /= n - 1;
         nx = 0;
         while (nx < n) {
                 int fid = pcreate(count);
                 if (fid < 0) break;
                 ids[nx++] = fid;
                 test_it();
         }
         assert(nx < n);
         for (i=0; i<nx; i++) {
                 assert(pdelete(ids[i]) == 0);
                 test_it();
         }
         printf(" > %d", nx);

         prio = getprio(getpid());
         assert(prio == 128);
         n = 0;
         while (1) {
                 int pid = start(no_run, 2000, 127, "no_run", 0);
                 if (pid < 0) break;
                 ids[n++] = pid;
                 if (n == l) {
                         assert(!"Maximum number of processes too high !");
                 }
                 test_it();
         }
         for (i=0; i<n; i++) {
                 assert(kill(ids[i]) == 0);
                 assert(waitpid(ids[i], 0) == ids[i]);
                 test_it();
         }
         for (i=0; i<n; i++) {
                 int pid = start(proc_return, 2000, 129, "proc_return", (void *)i);
                 assert(pid > 0);
                 ids[i] = pid;
                 test_it();
         }
         for (i=0; i<n; i++) {
                 int retval;
                 assert(waitpid(ids[i], &retval) == ids[i]);
                 assert(retval == i);
                 test_it();
         }
         printf(", %d.\n", n);
         return 0;
  }

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
#define NR_PHILO 5

 struct philo {
         char f[NR_PHILO]; /* tableau des fourchettes, contient soit 1 soit 0 selon si elle
                              est utilisee ou non */
         char bloque[NR_PHILO]; /* memorise l'etat du philosophe, contient 1 ou 0 selon que le philosophe
                                   est en attente d'une fourchette ou non */
         /* Padding pour satisfaire un compilateur strict. */
         char padding[sizeof(int) - (NR_PHILO * 2) % sizeof(int)];
         union sem mutex_philo; /* exclusion mutuelle */
         union sem s[NR_PHILO]; /* un semaphore par philosophe */
         int etat[NR_PHILO];
 };

 static int test_20(void *arg)
 {
         int j, pid;
         struct philo *p;

         (void)arg;

         p = (struct philo*) shm_create("shm_philo");
         assert(p != (void*)0);

         xscreate(&p->mutex_philo); /* semaphore d'exclusion mutuelle */
         xsignal(&p->mutex_philo);
         for (j = 0; j < NR_PHILO; j++) {
                 xscreate(p->s + j); /* semaphore de bloquage des philosophes */
                 p->f[j] = 0;
                 p->bloque[j] = 0;
                 p->etat[j] = '-';
         }

         printf("\n");
         pid = start(launch_philo, 4000, 193, "launch_philo", 0);
         assert(pid > 0);
         assert(waitpid(pid, 0) == pid);
         printf("\n");
         xsdelete(&p->mutex_philo);
         for (j = 0; j < NR_PHILO; j++) {
                 xsdelete(p->s + j);
         }
         shm_release("shm_philo");
         return 0;
 }

 static void affiche_etat(struct philo *p)
 {
         int i;
         printf("%c", 13);
         for (i=0; i<NR_PHILO; i++) {
                 unsigned long c;
                 switch (p->etat[i]) {
                         case 'm':
                                 c = 2;
                                 break;
                         default:
                                 c = 4;
                 }
                 (void)c;
                 printf("%c", p->etat[i]);
         }
 }

 static void waitloop(void)
 {
         int j;
         for (j = 0; j < 5000; j++) {
                 int l;
                 test_it();
                 for (l = 0; l < 5000; l++);
         }
 }

 static void penser(struct philo *p, long i)
 {
         xwait(&p->mutex_philo); /* DEBUT SC */
         p->etat[i] = 'p';
         affiche_etat(p);
         xsignal(&p->mutex_philo); /* Fin SC */
         waitloop();
         xwait(&p->mutex_philo); /* DEBUT SC */
         p->etat[i] = '-';
         affiche_etat(p);
         xsignal(&p->mutex_philo); /* Fin SC */
 }

 static void manger(struct philo *p, long i)
 {
         xwait(&p->mutex_philo); /* DEBUT SC */
         p->etat[i] = 'm';
         affiche_etat(p);
         xsignal(&p->mutex_philo); /* Fin SC */
         waitloop();
         xwait(&p->mutex_philo); /* DEBUT SC */
         p->etat[i] = '-';
         affiche_etat(p);
         xsignal(&p->mutex_philo); /* Fin SC */
 }

 static int test(struct philo *p, int i)
 {
         /* les fourchettes du philosophe i sont elles libres ? */
         return ((!p->f[i] && (!p->f[(i + 1) % NR_PHILO])));
 }

 static void prendre_fourchettes(struct philo *p, int i)
 {
         /* le philosophe i prend des fourchettes */

         xwait(&p->mutex_philo); /* Debut SC */

         if (test(p, i)) {  /* on tente de prendre 2 fourchette */
                 p->f[i] = 1;
                 p->f[(i + 1) % NR_PHILO] = 1;
                 xsignal(&p->s[i]);
         } else
                 p->bloque[i] = 1;

         xsignal(&p->mutex_philo); /* FIN SC */
         xwait(&p->s[i]); /* on attend au cas o on ne puisse pas prendre 2 fourchettes */
 }

 static void poser_fourchettes(struct philo *p, int i)
 {

         xwait(&p->mutex_philo); /* DEBUT SC */

         if ((p->bloque[(i + NR_PHILO - 1) % NR_PHILO]) && (!p->f[(i + NR_PHILO - 1) % NR_PHILO])) {
                 p->f[(i + NR_PHILO - 1) % NR_PHILO] = 1;
                 p->bloque[(i + NR_PHILO - 1) % NR_PHILO] = 0;
                 xsignal(&p->s[(i + NR_PHILO - 1) % NR_PHILO]);
         } else
                 p->f[i] = 0;

         if ((p->bloque[(i + 1) % NR_PHILO]) && (!p->f[(i + 2) % NR_PHILO])) {
                 p->f[(i + 2) % NR_PHILO] = 1;
                 p->bloque[(i + 1) % NR_PHILO] = 0;
                 xsignal(&p->s[(i + 1) % NR_PHILO]);
         } else
                 p->f[(i + 1) % NR_PHILO] = 0;

         xsignal(&p->mutex_philo); /* Fin SC */
 }

 static int philosophe(void *arg)
 {
         /* comportement d'un seul philosophe */
         int i = (int) arg;
         int k;
         struct philo *p;

         p = shm_acquire("shm_philo");
         assert(p != (void*)0);

         for (k = 0; k < 6; k++) {
                 prendre_fourchettes(p, i); /* prend 2 fourchettes ou se bloque */
                 manger(p, i); /* le philosophe mange */
                 poser_fourchettes(p, i); /* pose 2 fourchettes */
                 penser(p, i); /* le philosophe pense */
         }
         xwait(&p->mutex_philo); /* DEBUT SC */
         p->etat[i] = '-';
         affiche_etat(p);
         xsignal(&p->mutex_philo); /* Fin SC */
         shm_release("shm_philo");
         return 0;
 }

 static int launch_philo(void *arg)
 {
         int i, pid;

         (void)arg;

         for (i = 0; i < NR_PHILO; i++) {
                 pid = start(philosophe, 4000, 192, "philosophe", (void *) i);
                 assert(pid > 0);
         }
         return 0;
 }

/*-----------------*
 *      Test 21
 *-----------------*/
static int test_21(void *arg)
{
  (void)arg;
  char *shared_area = NULL;
  int   checker_pid = -1;
  int   checker_ret = -1;

  printf("\n%s\n", "Test 21: checking shared memory space ...");

  shared_area = shm_create("test21-shm");
  assert(shared_area != NULL);

  /* We have to be able to fill at least 1 page */
  memset(shared_area, FILL_VALUE, 4096);

  /* Let the check do its job */
  checker_pid =
      start(shm_checker, 4000, getprio(getpid()) - 1, "shm_checker", NULL);
  assert(checker_pid > 0);

  waitpid(checker_pid, &checker_ret);

  switch (checker_ret) {
  case CHECKER_SUCCESS:
    printf(
        " -> %s\n -> %s\n", "\"shm_checker\" ends correctly.", "TEST PASSED");
    break;
  case 0:
    printf(" -> %s\n -> %s\n", "\"shm_checker\" killed.", "TEST FAILED");
    break;
  default:
    printf(" -> %s\n -> %s\n",
           "\"shm_checker\" returned inconsistent value. Check waitpid "
           "implementation.",
           "TEST FAILED");
  }

  int shm_valid = 1;
  for (int i = 0; i < 4096; i++) {
    if (shared_area[i] != 0) {
      shm_valid = 0;
    }
  }

  if (shm_valid) {
    printf(" -> %s\n -> %s\n", "shm area content is correct.", "TEST PASSED");
  } else {
    printf(" -> %s\n -> %s\n", "shm area content is invalid.", "TEST FAILED");
  }

  shm_release("test21-shm");
  return 0;
}

static int shm_checker(void *arg)
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

/*-----------------*
 *      Test 22
 *-----------------*/
// TODO: Add test_22 when shared memory is available
// NOTE: test_22 assumes some kind of memory protection from kernel, irrelevant
// as no memory management is done until phase 5
