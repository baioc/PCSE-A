#include "start.h"

#include "mem.h"
#include "userspace_apps.h"
#include "kernel_tests.h"
#include "stdio.h"

/// Initialize console subsystem.
extern void console_init(void);

/// Initializes the system clock and timer interrupt handles.
extern void clock_init(void);

/// Initializes syscall handlers.
extern void syscall_init(void);

/// Initializes the process management subsystem and moves to process "idle".
extern void process_init(void);


void kernel_start(void)
{
  console_init();
  printf(":: reached kernel start \n");

  printf(":: running kernel tests\n");
  kernel_run_general_tests();

  printf(":: configuring system timer\n");
  clock_init();

  printf(":: initializing page allocator\n");
  mem_init();

  printf(":: loading user applications\n");
  uapps_init();

  printf(":: configuring syscall handlers\n");
  syscall_init();

  printf(":: starting process management subsystem\n");
  process_init();
}
