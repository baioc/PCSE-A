#include "stdio.h"
#include "debugger.h"
#include "cpu.h"
#include "console.h"
#include "interrupts.h"
#include "clock.h"

void kernel_start(void)
{
  // initialize subsystems
  printf("\f");
  clock_init();

  // testing
  int i = 42;
  printf("Hello world\n");
  printf("The answer is %d\n", i);
  printf("\n");
  // call_debugger();

  // enable interrupts and begin handler daemon
  sti();
  for (unsigned s = -1;;) {
    hlt();

    unsigned seconds = current_clock() / CLOCKFREQ;
    if (seconds == s) {
      continue;
    }
    s = seconds;
    unsigned minutes = seconds / 60;
    seconds %= 60;
    unsigned hours = minutes / 60;
    minutes %= 60;
    hours %= 24;
    printf("%02u:%02u:%02u\n", hours, minutes, seconds);
  }
}
