#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"
#include "sem.h"
#include "mqueue.h"
#include "shm.h"

int main(void)
{
  int pid, shell_pid, ret, msg;
  printf(":: reached init\n");
  printf(":: reached target user system\n");

  int  sid = screate(0);
  int  fid1 = pcreate(6);
  int  fid2 = pcreate(1);
  int *shell_mem = (int *)shm_create("shell-mem");
  shell_mem[0] = sid;
  shell_mem[1] = fid1;
  shell_mem[2] = fid2;

  printf(":: starting system shell\n");

  shell_pid = start("shell", 2048, 1, 0);

  wait(sid);

  preceive(fid1, &msg);
  psend(fid2, 20);
  psend(fid2, 10);

  // Once the command interpreter has exited it's time to shutdown
  while (waitpid(-1, NULL) != shell_pid)
    ;

  shm_release("shell_mem");

  exit(0);
}
