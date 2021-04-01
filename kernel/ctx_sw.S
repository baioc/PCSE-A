#include "segment.h"

	.text
	.globl ctx_sw
# Stack structure:
#   %esp + 4 : old process context address
#   %esp + 8 : new process context address
ctx_sw:
	# we don't need to save temp registers since this is a function call
	movl 4(%esp), %eax
	movl %ebx, (%eax)
	movl %esp, 4(%eax)
	movl %ebp, 8(%eax)
	movl %esi, 12(%eax)
	movl %edi, 16(%eax)
	movl %cr3, %ecx
	movl %ecx, 20(%eax)
	# load the new process context
	movl 8(%esp), %eax
	movl (%eax), %ebx
	movl 4(%eax), %esp
	movl 8(%eax), %ebp
	movl 12(%eax), %esi
	movl 16(%eax), %edi
	movl 20(%eax), %ecx
	movl %ecx, %cr3
	# yield kernel execution to the new proc' by returning with the new stack
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
	orl $0x0200, (%esp) # set IF in eflags to re-enable interrupts on iret
	pushl $USER_CS # CS
	pushl %eax     # EIP
	# store kernelspace CR3 in tss and set CR3 to userspace PDBR
	movl %cr3, %eax
	movl %eax, tss+28
	movl %ecx, %cr3 # this flushes the TLB for us
	# "return" (for the first time) to userspace
	iret
