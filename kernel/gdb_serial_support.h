#ifndef GDB_SERIAL_SUPPORT_H_
#define GDB_SERIAL_SUPPORT_H_

typedef unsigned long oskit_addr_t;
typedef unsigned char oskit_u8_t;
typedef unsigned long oskit_size_t;

/* This structure represents the x86 register state frame as GDB wants it.  */
struct gdb_state
{
	unsigned	eax;
	unsigned	ecx;
	unsigned	edx;
	unsigned	ebx;
	unsigned	esp;
	unsigned	ebp;
	unsigned	esi;
	unsigned	edi;
	unsigned	eip;
	unsigned	eflags;
	unsigned	cs;
	unsigned	ss;
	unsigned	ds;
	unsigned	es;
	unsigned	fs;
	unsigned	gs;
};

/*
 * Parameter block used by `gdb_serial_converse' and its callbacks.
 */
struct gdb_params
{
	/*
	 * An ID number for the current thread, -1 if no thread callbacks.
	 */
	oskit_addr_t thread_id;

	/*
	 * The signal number describing why the program stopped.
	 * On GDB_CONT return, signal to take when continuing or zero
	 * to continue execution normally.
	 */
	oskit_u8_t signo;

	/*
	 * Register state of the current thread.
	 * The caller must provide this buffer.
	 */
	struct gdb_state *regs;

	/*
	 * Set by `gdb_serial_converse' when values in `regs' have been
	 * changed.  The caller (or callback) then needs to commit these
	 * changes to the thread.
	 */
	int regs_changed;
};

/*
 * Return values from `gdb_serial_converse' (first one is never returned).
 * These indicate to the caller what the user has asked GDB to do.
 */
enum gdb_action {
	GDB_MORE,		/* internal - keeping reading gdb cmds */
	GDB_CONT,		/* continue with signo */
	GDB_RUN,		/* rerun the program from the beginning */
	GDB_KILL,		/* kill the program */
	GDB_DETACH		/* continue; debugger is going away */
};

/*
 * Structure of callbacks for `gdb_serial_converse'.
 * Any of these pointers may be null, indicating the facility is not available.
 */
struct gdb_callbacks
{
	/*
	 * Return zero iff `thread_id' matches a live thread.
	 */
	int (*thread_alive) (struct gdb_params *p, oskit_addr_t thread_id);

	/*
	 * Called only if `thread_id' differs from `p->thread_id'.
	 * If `p->regs_changed' is set, the now-current thread needs
	 * its register state sync'd with `p->regs'.  After that,
	 * switch to `thread_id' and fetch its register state into `p->regs'.
	 * Return zero if successful or an error code if `thread_id' is bogus.
	 */
	int (*thread_switch) (struct gdb_params *p, oskit_addr_t thread_id);
};

enum gdb_action
gdb_serial_converse (struct gdb_params *p, const struct gdb_callbacks *cb);

void
gdb_serial_exit(int exit_code);

#define NSIG 32

#define SIGHUP  1       /* hangup */
#define SIGINT  2       /* interrupt */
#define SIGQUIT 3       /* quit */
#define SIGILL  4       /* illegal instruction (not reset when caught) */
#define SIGTRAP 5       /* trace trap (not reset when caught) */
#define SIGABRT 6       /* abort() */
#define SIGIOT  SIGABRT /* compatibility */
#define SIGEMT  7       /* EMT instruction */
#define SIGFPE  8       /* floating point exception */
#define SIGKILL 9       /* kill (cannot be caught or ignored) */
#define SIGBUS  10      /* bus error */
#define SIGSEGV 11      /* segmentation violation */
#define SIGSYS  12      /* bad argument to system call */
#define SIGPIPE 13      /* write on a pipe with no one to read it */
#define SIGALRM 14      /* alarm clock */
#define SIGTERM 15      /* software termination signal from kill */
#define SIGURG  16      /* urgent condition on IO channel */
#define SIGSTOP 17      /* sendable stop signal not from tty */
#define SIGTSTP 18      /* stop signal from tty */
#define SIGCONT 19      /* continue a stopped process */
#define SIGCHLD 20      /* to parent on child stop or exit */
#define SIGTTIN 21      /* to readers pgrp upon background tty read */
#define SIGTTOU 22      /* like TTIN for output if (tp->t_local&LTOSTOP) */
#define SIGIO   23      /* input/output possible signal */
#define SIGXCPU 24      /* exceeded CPU time limit */
#define SIGXFSZ 25      /* exceeded file size limit */
#define SIGVTALRM 26    /* virtual time alarm */
#define SIGPROF 27      /* profiling time alarm */
#define SIGWINCH 28     /* window size changes */
#define SIGINFO 29      /* information request */
#define SIGUSR1 30      /* user defined signal 1 */
#define SIGUSR2 31      /* user defined signal 2 */

// On a multiprocessor machine, these functions should be defined
#define base_critical_enter() do { } while (0)
#define base_critical_leave() do { } while (0)

/*
 * These functions are provided to copy data
 * into and out of the address space of the program being debugged.
 * Our code provides default implementations that simply copy data
 * into and out of the kernel's own address space. If you want to
 * support separate address spaces, you have to change the code.
 * These functions return zero if the copy succeeds,
 * or nonzero if the memory region couldn't be accessed for some reason.
 */
int gdb_copyin(oskit_addr_t src_va, void *dest_buf, oskit_size_t size);
int gdb_copyout(const void *src_buf, oskit_addr_t dest_va, oskit_size_t size);
void copy_client_paging_registers(unsigned long cr0, unsigned long cr3, unsigned long cr4);

/*
 * The GDB stub calls this architecture-specific function
 * to modify the trace flag in the processor state.
 */
void gdb_set_trace_flag(int trace_enable, struct gdb_state *inout_state);

int (*gdb_serial_recv)(void);
void (*gdb_serial_send)(int ch);

#endif /*GDB_SERIAL_SUPPORT_H_*/
