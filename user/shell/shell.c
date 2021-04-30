#include "stdio.h"
#include "string.h"
#include "stddef.h"
#include "stdbool.h"

#include "process.h"
#include "sem.h"
#include "mqueue.h"
#include "console.h"

#define CMD_BUFFER_SIZE 256

static void help();
static void parse_cmd(char *cmd);
static void echo();

static bool has_echo;

int main(void)
{
  has_echo = true;

  int  nread;
  char cmd_buffer[CMD_BUFFER_SIZE];
  printf("Hey, I'm a shell. Type help for a list of available commands.\n");

  // Get first input
  printf("$ ");
  nread = cons_read(cmd_buffer, CMD_BUFFER_SIZE);
  cmd_buffer[nread] = '\0';

  while (strcmp(cmd_buffer, "exit") != 0) {
    parse_cmd(cmd_buffer);

    // Get next input
    printf("$ ");
    nread = cons_read(cmd_buffer, CMD_BUFFER_SIZE);
    cmd_buffer[nread] = '\0';
  }
  printf("exit\n");
}

static void parse_cmd(char *cmd)
{
  if (strcmp(cmd, "help") == 0) {
    help();
  } else if (strcmp(cmd, "ps") == 0) {
    ps();
  } else if (strcmp(cmd, "sinfo") == 0) {
    sinfo();
  } else if (strcmp(cmd, "pinfo") == 0) {
    pinfo();
  } else if (strcmp(cmd, "echo") == 0) {
    echo();
  } else {
    // not a known internal command
    int bg = 0;

    // check if process should be run in 'background'
    char *c = cmd;
    while (*c != ' ' && *c != '\0') c++;

    if (*c == ' ' && *(c + 1) == '&') {
      bg = 1;
      *c = '\0';
    };

    int pid = start(cmd, 2048, 128, NULL);

    if (pid < 0) {
      printf("%s: Unknown command\n", cmd);
    } else if (bg) {
      disown(pid);
    } else {
      waitpid(pid, NULL);
    }
  }
}

static void echo()
{
  cons_echo(!has_echo);
  if (has_echo) {
    printf("Echo removed\n");
  } else {
    printf("\nEcho restored\n");
  }
  has_echo = !has_echo;
}

static void help()
{
  printf("Commands:\n");
  printf("- help:\t\tshow this help\n");
  printf("- ps:\t\tthe list of all created processes\n");
  printf("- sinfo:\tthe list of all created semaphores\n");
  printf("- pinfo:\tthe list of all created message queues\n");
  printf("- echo:\t\ttoggle character printing in console\n");
  printf("- exit:\t\texit kernel (and reboot)\n");
  printf("- any existing process (for instance \"autotest\", in background "
         "\"autotest &\")\n");
}
