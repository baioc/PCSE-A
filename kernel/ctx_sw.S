    .text
    .globl ctx_sw
# Stack structure:
#   %esp + 4 : old process context address
#   %esp + 8 : new process context address
# We suppose this is being called from an interrupt such that %eflags was saved
# when the interrupt handler began and will be restored on `iret`.
ctx_sw:
    # we don't need to save %eax, %edx or %ecx since this is a function call
    movl 4(%esp), %eax
    movl %ebx, (%eax)
    movl %esp, 4(%eax)
    movl %ebp, 8(%eax)
    movl %esi, 12(%eax)
    movl %edi, 16(%eax)
    # load the new process context
    movl 8(%esp), %eax
    movl (%eax), %ebx
    movl 4(%eax), %esp
    movl 8(%eax), %ebp
    movl 12(%eax), %esi
    movl 16(%eax), %edi
    # make sure interrupts are enabled before yielding
    sti
    # give the cpu to the process by returning to it with the updated %esp
    ret


    .text
    .globl proc_exit
# The sole reason this needs to be in assembly is to get the return value from
# the exiting process' main routine and pass it correctly to the real exit().
proc_exit:
    pushl %eax
    jmp exit
