#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/// Software interrupt number to trigger syscalls.
#define NR_SYSCALL 49

/// Syscall number to execute, must be in EAX before the interrupt.
#define NR_cons_write     0
#define NR_clock_settings 3
#define NR_current_clock  4
#define NR_wait_clock     5
#define NR_chprio         6
#define NR_exit           7
#define NR_getpid         8
#define NR_getprio        9
#define NR_kill           10
#define NR_start          11
#define NR_waitpid        12

#endif // _SYSCALL_H_
