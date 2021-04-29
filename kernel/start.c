#include "start.h"

#include "kbd.h"
#include "mem.h"
#include "userspace_apps.h"
#include "stdio.h"

/// Initialize console subsystem.
extern void console_init(void);

/// Initialize the system clock and timer interrupt handles.
extern void clock_init(void);

/// Starts process management system.
extern void process_init(void);

/// Starts special idle process, which moves to userspace init.
extern void idle(void);

/// Initialize syscall handlers.
extern void syscall_init(void);

/// Initialize message queue management subsystem.
extern void mq_init(void);

/// Initialize semaphores.
extern void sem_init(void);

/// Initializes shared memory.
extern void shm_init(void);


void kernel_start(void)
{
  console_init();

  printf(":: configuring system timer\n");
  clock_init();

  printf(":: initializing page allocator\n");
  mem_init();

  printf(":: initializing shared memory subsystem\n");
  shm_init();

  printf(":: loading user applications\n");
  uapps_init();

  printf(":: initializing process scheduler\n");
  process_init();

  printf(":: initializing message queues \n");
  mq_init();

  printf(":: initializing semaphores\n");
  sem_init();

  printf(":: configuring PS/2 keyboard driver\n");
  kbd_init();

  printf(":: configuring syscall handlers\n");
  syscall_init();

  printf(":: switching to userspace\n");
  idle();
}
