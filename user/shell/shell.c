#include "stdio.h"
#include "process.h"
#include "sem.h"
#include "mqueue.h"
#include "shm.h"

static void help();

int main()
{
  int *shell_mem = (int *)shm_acquire("shell-mem");
  int  msg;
  printf("Hey, I'm a shell. Type help for a list of available commands.\n");
  printf("$ ");

  // TODO: Once keyboard is done, we'll use cons_read syscall to get user input
  // parse that input and then launch the appropriate command. If no internal
  // command corresponds to that input, then we'll try to start a process
  // corresponding to that input
  printf("\nCommands showcase\n");

  printf("**help command\n**");
  help();

  printf("**ps command**\n");
  ps();

  int sid = screate(1);

  printf("**sinfo command**\n");
  sinfo();

  sdelete(sid);
  sdelete(shell_mem[0]);

  printf("**pinfo command**\n");
  pinfo();

  psend(shell_mem[1], 15);
  pdelete(shell_mem[1]);

  printf("**pinfo command (round 2)**\n");
  pinfo();

  preceive(shell_mem[2], &msg);
  pdelete(shell_mem[2]);

  shm_release("shell_mem");
}

static void help()
{
  printf("Commands:\n");
  printf("- help:\t\tshow this help\n");
  printf("- ps:\t\tthe list of all created processes\n");
  printf("- sinfo:\tthe list of all created semaphores\n");
  printf("- pinfo:\t\tthe list of all created message queues\n");
  printf("- exit:\t\texit kernel (and reboot)\n");
}
