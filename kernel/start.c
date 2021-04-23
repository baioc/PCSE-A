#include "start.h"

#include "mem.h"
#include "userspace_apps.h"
#include "stdio.h"

/// Initialize console subsystem.
extern void console_init(void);

/// Initialize the system clock and timer interrupt handles.
extern void clock_init(void);

/// Initialize syscall handlers.
extern void syscall_init(void);

/// Initialize message queue management subsystem.
extern void mq_init(void);

/// Initialize semaphores.
extern void sem_init(void);

/// Initializes shared memory.
extern void shm_init(void);

/// Starts process management system and moves to process "idle".
extern void process_init(void);

void kernel_start(void)
{
  console_init();
  printf(":: reached kernel start \n");

  printf(":: configuring system timer\n");
  clock_init();

  printf(":: initializing page allocator\n");
  mem_init();

  printf(":: loading user applications\n");
  uapps_init();

  printf(":: configuring syscall handlers\n");
  syscall_init();

  printf(":: initializing message queues \n");
  mq_init();

  printf(":: initializing shared memory subsystem\n");
  shm_init();

  printf(":: initializing semaphores\n");
  sem_init();

  printf(":: starting process scheduler\n");
  process_init();
}
