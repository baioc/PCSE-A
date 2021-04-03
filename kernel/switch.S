#include "segment.h"

	.text
	.globl switch_context
# 4(%esp) : old process context address
# 8(%esp) : new process context address
switch_context:
	# we only need to save non-scratch regs
	movl 4(%esp), %eax
	movl %ebx, 0(%eax)
	movl %esp, 4(%eax)
	movl %ebp, 8(%eax)
	movl %esi, 12(%eax)
	movl %edi, 16(%eax)
	movl %cr3, %ecx
	movl %ecx, 20(%eax)
	movl tss+4, %ecx # tss+4 : TSS.ESP0
	movl %ecx, 24(%eax)
	# load new process context
	movl 8(%esp), %eax
	movl 0(%eax), %ebx
	movl 4(%eax), %esp
	movl 8(%eax), %ebp
	movl 12(%eax), %esi
	movl 16(%eax), %edi
	movl 20(%eax), %ecx
	movl %ecx, %cr3 # flushes the TLB
	movl %ecx, tss+28 # tss+28 : TSS.CR3
	movl 24(%eax), %ecx
	movl %ecx, tss+4
	# yield execution to the next process' by returning with the new stack
	ret

	.text
	.globl switch_mode_user
# 4(%esp)  : instruction in userspace we'll return to, virtual address
# 8(%esp)  : userspace stack pointer to restore, virtual address
# 12(%esp) : userspace pagedir pointer, real address
switch_mode_user:
	# set data segment selectors
	mov $USER_DS, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	# re-create interrupt stack frame
	movl 4(%esp), %eax  # eax <- EIP
	movl 8(%esp), %edx  # edx <- ESP
	movl 12(%esp), %ecx # ecx <- CR3
	pushl $USER_DS # SS
	pushl %edx     # ESP
	pushfl         # EFLAGS
	orl $0x0200, (%esp)      # set IF to get enabled interrupts on iret
	andl $0xFFFFCFFF, (%esp) # also make sure IOPL is set to 0
	pushl $USER_CS # CS
	pushl %eax     # EIP
	# "return" (for the first time) to userspace
	iret
