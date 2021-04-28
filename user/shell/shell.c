#include "stdio.h"
#include "process.h"
#include "sem.h"

static void help();

int main()
{
  printf("Hey, I'm a shell. Type help for a list of available commands.\n");
  printf("$ ");
  printf("\nCommands showcase\n");

  printf("**help command\n**");
  help();

  printf("**ps command**\n");
  ps();

  int sid = screate(1);

  printf("**sinfo command**\n");
  sinfo();

  sdelete(sid);
}

static void help()
{
  printf("Commands:\n");
  printf("- help:\t\tshow this help\n");
  printf("- ps:\t\tthe list of all created processes\n");
  printf("- sinfo:\tthe list of all created semaphores\n");
}
