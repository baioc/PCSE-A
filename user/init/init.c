#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"

int main(void)
{
  int shell_pid;

  printf(":: reached init\n");
  printf(":: reached target user system\n");

  printf(":: starting system shell\n");
  shell_pid = start("shell", 2048, 1, 0);

  // Once the command interpreter has exited it's time to shutdown
  while (waitpid(-1, NULL) != shell_pid)

  return 0;
}
