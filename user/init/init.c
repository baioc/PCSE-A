#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"
#include "sem.h"

int main(void)
{
  int pid, shell_pid, ret;
  printf(":: reached init\n");
  printf(":: reached target user system\n");

  printf(":: starting system shell\n");
  int sid = screate(0);

  shell_pid = start("shell", 2048, 1, (void *)sid);

  wait(sid);

  // Once the command interpreter has exited it's time to shutdown
  while (waitpid(-1, NULL) != shell_pid)
    ;

  exit(0);
}
