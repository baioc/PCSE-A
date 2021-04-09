#define SERV_EXC_TASK(id) \
	.text; \
trap##id: \
	pushl	$(0); \
	pushl	$(id); \
	cld;	/* for gcc */ \
	call	trap_handler; \
	addl	$(8),%esp; \
	iret; \
	jmp	trap##id; \
	.data; \
	.long	trap##id

#define SERV_ERR_TASK(id) \
	.text; \
trap##id: \
	pushl	$(id); \
	cld;	/* for gcc */ \
	call	trap_handler; \
	addl	$(8),%esp; \
	iret; \
	jmp	trap##id; \
	.data; \
	.long	trap##id

	.data
.globl exception_handler_tasks
exception_handler_tasks:
SERV_EXC_TASK(0)
SERV_EXC_TASK(1)
SERV_EXC_TASK(2)
SERV_EXC_TASK(3)
SERV_EXC_TASK(4)
SERV_EXC_TASK(5)
SERV_EXC_TASK(6)
SERV_EXC_TASK(7)
SERV_ERR_TASK(8)
SERV_EXC_TASK(9)
SERV_ERR_TASK(10)
SERV_ERR_TASK(11)
SERV_ERR_TASK(12)
SERV_ERR_TASK(13)
SERV_ERR_TASK(14)
SERV_EXC_TASK(15)
SERV_EXC_TASK(16)
SERV_ERR_TASK(17)
SERV_EXC_TASK(18)
SERV_EXC_TASK(19)
SERV_EXC_TASK(20)
SERV_EXC_TASK(21)
SERV_EXC_TASK(22)
SERV_EXC_TASK(23)
SERV_EXC_TASK(24)
SERV_EXC_TASK(25)
SERV_EXC_TASK(26)
SERV_EXC_TASK(27)
SERV_EXC_TASK(28)
SERV_EXC_TASK(29)
SERV_EXC_TASK(30)
SERV_EXC_TASK(31)


/* NOTE: these handlers are evoked by intr gates, so interrupts are disabled */
#define DEFINE_INTERRUPT_HANDLER(FUNCTION)                                   \
	.text;                                                                   \
	.globl FUNCTION##_handler;                                               \
FUNCTION##_handler:                                                          \
	/* call actual handler (also save all scratch registers) */              \
	pushl %eax;                                                              \
	pushl %edx;                                                              \
	pushl %ecx;                                                              \
	call FUNCTION;                                                           \
	popl %ecx;                                                               \
	popl %edx;                                                               \
	popl %eax;                                                               \
	/* return re-enables interrupts */                                       \
	iret

DEFINE_INTERRUPT_HANDLER(clock_tick) /* clock.c */
DEFINE_INTERRUPT_HANDLER(kbd_interrupt) /* kbd.c */

/* XXX: these may push an error code into the stack, but since we only handle
faults to terminate a process, we don't need to worry about popping the error
code before returning: we never return. handlers defined in process.c */
DEFINE_INTERRUPT_HANDLER(divide_error)
DEFINE_INTERRUPT_HANDLER(protection_exception)
DEFINE_INTERRUPT_HANDLER(page_fault)


/* Syscall parameter handler: simply passes reified args to do_syscall() */
	.text
	.globl syscall_handler
syscall_handler:
	/* push args in reverse order */
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	call do_syscall /* return code is in EAX */
	add $(6*4), %esp /* remember to restore stack frame */
	iret
