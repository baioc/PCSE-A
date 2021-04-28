#include "stdio.h"
#include "process.h"

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
}

static void help()
{
  printf("Commands:\n");
  printf("- help:\tshow this help\n");
  printf("- ps:\tthe list of all created processes\n");
}
