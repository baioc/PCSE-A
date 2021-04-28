#include "syscall.h"

/* Templates for each arity (0 to 5): */

#define DEFN_SYSCALL0(FN) \
	.text;                \
	.globl FN;            \
FN:;                      \
	movl $NR_##FN, %eax;  \
	int $NR_SYSCALL;      \
	ret

#define DEFN_SYSCALL1(FN) \
	.text;                \
	.globl FN;            \
FN:;                      \
	movl $NR_##FN, %eax;  \
	pushl %ebx;           \
	movl 8(%esp), %ebx;   \
	int $NR_SYSCALL;      \
	popl %ebx;            \
	ret

#define DEFN_SYSCALL2(FN) \
	.text;                \
	.globl FN;            \
FN:;                      \
	movl $NR_##FN, %eax;  \
	pushl %ebx;           \
	movl 8(%esp), %ebx;   \
	movl 12(%esp), %ecx;  \
	int $NR_SYSCALL;      \
	popl %ebx;            \
	ret

#define DEFN_SYSCALL3(FN) \
	.text;                \
	.globl FN;            \
FN:;                      \
	movl $NR_##FN, %eax;  \
	pushl %ebx;           \
	movl 8(%esp), %ebx;   \
	movl 12(%esp), %ecx;  \
	movl 16(%esp), %edx;  \
	int $NR_SYSCALL;      \
	popl %ebx;            \
	ret

#define DEFN_SYSCALL4(FN) \
	.text;                \
	.globl FN;            \
FN:;                      \
	movl $NR_##FN, %eax;  \
	pushl %ebx;           \
	movl 8(%esp), %ebx;   \
	movl 12(%esp), %ecx;  \
	movl 16(%esp), %edx;  \
	pushl %esi;           \
	movl 24(%esp), %esi;  \
	int $NR_SYSCALL;      \
	popl %esi;            \
	popl %ebx;            \
	ret

#define DEFN_SYSCALL5(FN) \
	.text;                \
	.globl FN;            \
FN:;                      \
	movl $NR_##FN, %eax;  \
	pushl %ebx;           \
	movl 8(%esp), %ebx;   \
	movl 12(%esp), %ecx;  \
	movl 16(%esp), %edx;  \
	pushl %esi;           \
	movl 24(%esp), %esi;  \
	pushl %edi;           \
	movl 32(%esp), %edi;  \
	int $NR_SYSCALL;      \
	popl %edi;            \
	popl %esi;            \
	popl %ebx;            \
	ret


/* Definitions: */

/* console.h */
DEFN_SYSCALL2(cons_write)
DEFN_SYSCALL2(cons_read)
DEFN_SYSCALL1(cons_echo)

/* clock.h */
DEFN_SYSCALL2(clock_settings)
DEFN_SYSCALL0(current_clock)
DEFN_SYSCALL1(wait_clock)

/* process.h */
DEFN_SYSCALL4(start)
DEFN_SYSCALL0(getpid)
DEFN_SYSCALL1(getprio)
DEFN_SYSCALL2(chprio)
DEFN_SYSCALL1(kill)
DEFN_SYSCALL2(waitpid)
DEFN_SYSCALL1(exit)

/* mqueue.h */
DEFN_SYSCALL2(pcount)
DEFN_SYSCALL1(pcreate)
DEFN_SYSCALL1(pdelete)
DEFN_SYSCALL2(preceive)
DEFN_SYSCALL1(preset)
DEFN_SYSCALL2(psend)

/* sem.h */
DEFN_SYSCALL1(scount)
DEFN_SYSCALL1(screate)
DEFN_SYSCALL1(sdelete)
DEFN_SYSCALL1(signal)
DEFN_SYSCALL2(signaln)
DEFN_SYSCALL2(sreset)
DEFN_SYSCALL1(try_wait)
DEFN_SYSCALL1(wait)

/* smh.h */
DEFN_SYSCALL1(shm_create)
DEFN_SYSCALL1(shm_acquire)
DEFN_SYSCALL1(shm_release)
