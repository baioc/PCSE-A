#include "start.h"

#include "mem.h"
#include "userspace_apps.h"

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
  clock_init();
  mem_init();
  uapps_init();
  syscall_init();
  process_init();
}
