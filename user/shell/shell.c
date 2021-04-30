#include "stdio.h"
#include "string.h"
#include "stddef.h"
#include "stdbool.h"

#include "process.h"
#include "sem.h"
#include "mqueue.h"
#include "console.h"

#define CMD_BUFFER_SIZE 256

static void help(void);
static void parse_cmd(char *cmd);
static void echo(void);

static bool has_echo;

int main(void)
{
  has_echo = true;

  printf("  [~]      ____      _                _  ___  ____           \n"
         "  |=|     / ___|__ _| |_   ____ _  __| |/ _ \\/ ___|         \n"
         "  |=|    | |   / _` | \\ \\ / / _` |/ _` | | | \\___ \\      \n"
         ".-' '-.  | |__| (_| | |\\ V / (_| | (_| | |_| |___) |        \n"
         "|-----|   \\____\\__,_|_| \\_/ \\__,_|\\__,_|\\___/|____/    \n"
         "| ~~~ |                                                      \n"
         "| ~~~ |  Equipe: Maxime MARTIN, Gabriel B. SANT'ANNA,        \n"
         "|-----|  Thibault CANTORI, Pierre RAVENEL, Antoine BRIANCON  \n"
         "'-----'                                                      \n"
         "\tType \"help\" for a list of available commands.\n");

  for (char cmd_buffer[CMD_BUFFER_SIZE];;) {
    printf("\n$ ");
    unsigned long nread = cons_read(cmd_buffer, CMD_BUFFER_SIZE);
    cmd_buffer[nread] = '\0';
    if (nread == 0)
      continue;
    else if (strcmp(cmd_buffer, "exit") == 0)
      break;
    else
      parse_cmd(cmd_buffer);
  }
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
  } else { // not a known internal command
    // check if process should be run in 'background'
    bool  bg = false;
    char *c = cmd;
    while (*c != ' ' && *c != '\0') c++;
    if (*c == ' ' && *(c + 1) == '&') {
      bg = true;
      *c = '\0';
    };

    int pid = start(cmd, 2048, 128, NULL);
    if (pid < 0)
      printf("%s: Unknown command\n", cmd);
    else if (bg)
      disown(pid);
    else
      waitpid(pid, NULL);
  }
}

static void echo(void)
{
  cons_echo(!has_echo);
  if (has_echo) {
    printf("Echo removed\n");
  } else {
    printf("\nEcho restored\n");
  }
  has_echo = !has_echo;
}

static void help(void)
{
  printf("Commands:\n");
  printf("- help:\t\tshow this help message\n");
  printf("- ps:\t\tlist all created processes\n");
  printf("- sinfo:\tlist all created semaphores\n");
  printf("- pinfo:\tlist all created message queues\n");
  printf("- echo:\t\ttoggle character printing in the console\n");
  printf("- exit:\t\tpower off the system\n");
  printf("- any existing process "
         "(for instance \"sbrktest\", in background \"autotest &\")\n");
}
