#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"
#include "sem.h"

int main(void)
{
  int pid, ret;
  printf(":: reached init\n");
  printf(":: reached target user system\n");

  printf(":: starting system shell\n");
  pid = start("shell", 2048, 1, NULL);

  int sid = screate(0);
  wait(sid);

  for (;;) waitpid(-1, NULL);
}
