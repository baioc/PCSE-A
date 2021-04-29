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
#define NR_pcount         13
#define NR_pcreate        14
#define NR_pdelete        15
#define NR_preceive       16
#define NR_preset         17
#define NR_psend          18
#define NR_scount         19
#define NR_screate        20
#define NR_sdelete        21
#define NR_sreset         22
#define NR_signal         23
#define NR_signaln        24
#define NR_try_wait       25
#define NR_wait           26
#define NR_shm_create     27
#define NR_shm_acquire    28
#define NR_shm_release    29
// Syscalls principally used by shell
#define NR_ps    33
#define NR_sinfo 34
#define NR_pinfo 35

#endif // _SYSCALL_H_
