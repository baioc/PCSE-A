#include "stdio.h"
#include "process.h"
#include "sem.h"

static void help();

int main(void *arg)
{
  int init_sid = (int)arg;
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
  sdelete(init_sid);
}

static void help()
{
  printf("Commands:\n");
  printf("- help:\t\tshow this help\n");
  printf("- ps:\t\tthe list of all created processes\n");
  printf("- sinfo:\tthe list of all created semaphores\n");
  printf("- exit:\t\texit kernel (and reboot)\n");
}
