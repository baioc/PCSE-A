#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/// Software interrupt number to trigger syscalls.
#define NR_SYSCALL 49

/**
 * Syscall number to execute, must be in EAX before the interrupt.
 * Each system primitive is numbered as in the index table of
 * https://ensiwiki.ensimag.fr/index.php?title=Projet_système_:_spécification
 */
#define NR_console_putbytes 0
#define NR_chprio           1
#define NR_clock_settings   2
#define NR_current_clock    6
#define NR_exit             7
#define NR_getpid           8
#define NR_getprio          9
#define NR_kill             10
#define NR_start            17
#define NR_wait_clock       18
#define NR_waitpid          19

#endif // _SYSCALL_H_
