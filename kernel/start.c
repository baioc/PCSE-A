#include "stdio.h"
#include "debugger.h"
#include "cpu.h"
#include "console.h"
#include "interrupts.h"
#include "clock.h"
#include "process.h"

void kernel_start(void)
{
  // initialize subsystems
  console_init();
  clock_init();

  // testing
  int i = 42;
  printf("Hello world\n");
  printf("The answer is %d\n", i);
  // call_debugger();

  // enable interrupts and begin handler daemon
  sti();
  // prints elapsed time for 5 seconds
  char time[] = "HH:MM:SS";
  for (int i = 0; i < CLOCKFREQ*5; i++) {
    hlt();

    unsigned seconds = current_clock() / CLOCKFREQ;
    unsigned minutes = seconds / 60;
    seconds %= 60;
    unsigned hours = minutes / 60;
    minutes %= 60;
    hours %= 24;
    sprintf(time, "%02u:%02u:%02u", hours, minutes, seconds);
    console_write_raw(time, 8, 24, 72);
  }
  // deactivate interrupts
  cli();


  // Testing if 3 processes with the same priority can work together
  process_init();
}
