#include "stdio.h"
#include "string.h"
#include "stddef.h"
#include "process.h"
#include "sem.h"
#include "mqueue.h"

#define CMD_BUFFER_SIZE 256

static void help();
static void parse_cmd(char *cmd);

int main()
{
  char cmd_buffer[CMD_BUFFER_SIZE];
  printf("Hey, I'm a shell. Type help for a list of available commands.\n");

  printf("$ ");
  cons_read(cmd_buffer, CMD_BUFFER_SIZE);
  while (strcmp(cmd_buffer, "exit") != 0) {
    parse_cmd(cmd_buffer);
    cons_read(cmd_buffer, CMD_BUFFER_SIZE);
  }
  printf("exit\n");
}

static void parse_cmd(char *cmd)
{
  if (strcmp(cmd, "help") == 0) {
    help();
  } else if (strcmp(cmd, "ps")) {
    ps();
  } else if (strcmp(cmd, "sinfo")) {
    sinfo();
  } else if (strcmp(cmd, "pinfo")) {
    pinfo();
  } else {
    // not a known internal command
    int pid = start(cmd, 2048, 128, NULL);
    if (pid < 0) {
      printf("%s: Unknown command\n", cmd);
    }
  }
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
