/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Unique header of the standalone test standard library.
 */

#ifndef _SYSAPI_H_
#define _SYSAPI_H_

#define NULL ((void*)0)

/*******************************************************************************
 * Gestion de liste d'arguments de taille variable (printf)
 ******************************************************************************/
typedef void *__gnuc_va_list;
typedef __gnuc_va_list va_list;
#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_start(AP, LASTARG)                                           \
 (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))
#define va_end(AP)      ((void)0)

/*******************************************************************************
 * Printf macros
 ******************************************************************************/
#define PRINTF_LEFT_JUSTIFY 1
#define PRINTF_SHOW_SIGN 2
#define PRINTF_SPACE_PLUS 4
#define PRINTF_ALTERNATE 8
#define PRINTF_PAD0 16
#define PRINTF_CAPITAL_X 32

#define PRINTF_BUF_LEN 512

/*******************************************************************************
 * Assert : check a condition or fail
 ******************************************************************************/
#define __STRING(x) #x

#define assert(cond) \
((void)((cond) ? 0 : assert_failed(__STRING(cond), __FILE__, __LINE__)))

#define DUMMY_VAL 78

#define TSC_SHIFT 8

#define FREQ_PREC 50

#define NBSEMS 10000

#define TRUE 1
#define FALSE 0

#define NR_PHILO 5
// Prototype des appels systeme de la spec
int chprio(int pid, int newprio);
void cons_write(const char *str, unsigned long size);
#if defined CONS_READ_LINE
unsigned long cons_read(char *string, unsigned long length);
#elif defined CONS_READ_CHAR
int cons_read(void);
#endif
void cons_echo(int on);
void exit(int retval);
int getpid(void);
int getprio(int pid);
int kill(int pid);
#if defined WITH_SEM
int scount(int sem);
int screate(short count);
int sdelete(int sem);
int signal(int sem);
int signaln(int sem, short count);
int sreset(int sem, short count);
int try_wait(int sem);
int wait(int sem);
#elif defined WITH_MSG
int pcount(int fid, int *count);
int pcreate(int count);
int pdelete(int fid);
int preceive(int fid,int *message);
int preset(int fid);
int psend(int fid, int message);
#else
# error "WITH_SEM" ou "WITH_MSG" doit être définie.
#endif

/* Wrapper sur les verrous basés sur les sémaphores ou les files de messages */
union sem {
    int fid;
    int sem;
};
void xwait(union sem *s);
void xsignal(union sem *s);
void xsdelete(union sem *s);
void xscreate(union sem *s);

#ifndef TELECOM_TST
void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock(void);
void wait_clock(unsigned long wakeup);
#endif
int start(const char *process_name, unsigned long ssize, int prio, void *arg);
int waitpid(int pid, int *retval);

#if defined WITH_SEM
/*
 * Pour la soutenance, devrait afficher la liste des processus actifs, des
 * semaphores utilises et toute autre info utile sur le noyau.
 */
#elif defined WITH_MSG
/*
 * Pour la soutenance, devrait afficher la liste des processus actifs, des
 * files de messages utilisees et toute autre info utile sur le noyau.
 */
#endif
void sys_info(void);

/* Available from our standard library */
#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

typedef __SIZE_TYPE__ size_t;

int strcmp(const char *str1, const char *str2);
unsigned long strlen(const char *s);
char *strncpy(char *dst, const char *src, unsigned n);
void *memset(void *dst, int c, size_t n);

/* printf.h */
#define printf safe_printf
int safe_printf(const char *format, ...);
void cons_gets(char *s, unsigned long length);

/* assert.c */
int assert_failed(const char *cond, const char *file, int line);

/* math.h */
typedef unsigned long long uint_fast64_t;
typedef unsigned long uint_fast32_t;
short randShort(void);
void setSeed(uint_fast64_t _s);
unsigned long rand();
unsigned long long div64(unsigned long long num, unsigned long long div, unsigned long long *rem);

/* it.c */
void test_it(void);

/* Shared memory */
void *shm_create(const char*);
void *shm_acquire(const char*);
void shm_release(const char*);

#endif /* _SYSAPI_H_ */
