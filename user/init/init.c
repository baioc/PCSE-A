#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"

int main(void)
{
  int pid, shell_pid;
  printf(":: reached init\n");

  printf(":: testing dynamic user heap\n");
  pid = start("sbrktest", 0x1B0B, 1, NULL);
  assert(pid > 0);
  waitpid(pid, NULL);

  printf(":: reached target user system\n");
  printf(":: starting system shell\n");

  shell_pid = start("shell", 2048, 128, 0);

  // Once the command interpreter has exited it's time to shutdown
  while (waitpid(-1, NULL) != shell_pid)
    ;

  // Wait for remaining children
  while (waitpid(-1, NULL) > 0)
    ;

  ps();

  return 0;
}
