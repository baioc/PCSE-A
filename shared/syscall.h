#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/// Software interrupt number to trigger syscalls.
#define NR_SYSCALL 49

/// Syscall number to execute, must be in EAX before the interrupt.
#define NR_console_putbytes 0

#endif // _SYSCALL_H_
