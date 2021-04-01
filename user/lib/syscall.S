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

DEFN_SYSCALL2(console_putbytes)
