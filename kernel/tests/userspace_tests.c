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
#include "message-queue.h"
#include "debug.h"

#include "test1.h"
#include "test4.h"
#include "test13.h"

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

/*static int                test_8(void *arg);
static int                suicide_launcher(void *arg);
static int                suicide(void *arg);
static unsigned long long div64(unsigned long long x, unsigned long long div,
                                unsigned long long *rem);*/

static int test_12_msg(void *arg);
static int rdv_proc_12_msg(void *arg);

/*static int test_13_msg(void *arg);
static int p_receiver_13_msg(void *arg);
static int p_sender_13_msg(void *arg);*/

 static int test_14_msg(void *arg);
 static int psender_14_1(void *arg);
 static int psender_14_2(void *arg);

 static int test_15_msg(void *arg);
 static int psmg_15_1(void *arg);
 static int psmg_15_2(void *arg);

/*******************************************************************************
 * Variables
 ******************************************************************************/

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
  /*pid = start(test_8, 0, 128, "test_8", 0);
  waitpid(pid, NULL);*/
  pid = start(test_12_msg, 0, 128, "test_12_msg", 0);
  waitpid(pid, NULL);
  //start(test_13_msg, 0, 128, "test_13_msg", 0);
  pid = start(test_14_msg, 0, 128, "test_14_msg", 0);
  waitpid(pid, NULL);
  pid = start(test_15_msg, 0, 128, "test_15_msg", 0);
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

 /*static int p_receiver_13_msg(void *arg)
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
 }*/

/*-----------------*
 *      Test 14
 *-----------------*/

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
         print_list(fid1);

         print_waiting_send_proc(fid1);
         print_waiting_receive_proc(fid1);
         assert(preset(fid1) == 0);
         print_list(fid1);
         print_waiting_send_proc(fid1);
         print_waiting_receive_proc(fid1);

         assert((preceive(fid1, &msg) == 0) && (msg == 7));
         printf(" 7 t14");
         print_list(fid1);
         print_waiting_send_proc(fid1);
         print_waiting_receive_proc(fid1);
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
 /*******************************************************************************
  * Test 15
  *
  * Tuer des processus en attente sur file
  ******************************************************************************/

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
