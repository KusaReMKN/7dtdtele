#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <regex.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mknconf.h"

static char *configfile = NULL;
static const char *hostname = "localhost";
static unsigned short port = 8081;
static const char *fifoname = "/tmp/7dtdtelemod.p";

static int ParseConfigFile(const char *);

static void TermHandler(int);
static void RemoveFIFO(void);

static void DisplayVersion(void);
static void DisplayHelp(const char *);

int
main(int argc, char *argv[])
{
  // option check
  struct option longopts[] = {
    { "configfile", required_argument, NULL, 'c' },
    { "help",       no_argument,       NULL, 'h' },
    { "version",    no_argument,       NULL, 'v' },
    { NULL,         0          ,       NULL, 0   },
  };
  int opt;
  opterr = !0;
  while ((opt = getopt_long(argc, argv, "c:hv", longopts, NULL)) != -1) {
    switch (opt) {
    case 'c':
      configfile = optarg;
      break;
    case 'h':
      DisplayHelp(argv[0]);
      exit(EXIT_SUCCESS);
      /* NOTREACHED */
    case 'v':
      DisplayVersion();
      exit(EXIT_SUCCESS);
      /* NOTREACHED */
    case '?':
      fprintf(stderr, "More help, %1$s -h\n", argv[0]);
      exit(EXIT_FAILURE);
      /* NOTREACHED */
    }
  }

  if (configfile == NULL) {
    fprintf(stderr, "Config file is not specified.\n");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  // signal handler for SIGTERM
  struct sigaction act;
  act.sa_handler = TermHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  if (sigaction(SIGTERM, &act, NULL) < 0) {
    perror("sigaction -- SIGTERM");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  // SIGINT
  act.sa_handler = TermHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("sigaction -- SIGINT");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  // remove fifo at exit
  atexit(RemoveFIFO);

  // make FIFO
  if (mkfifo(fifoname, 0600) != 0) {
    perror("mkfifo");
    if (errno == EEXIST) {
      fprintf(stderr, "daemon is already running?\n");
    }
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  MKNDIC *dic;
  // Check Config file
  if ((dic = LoadConfig(configfile)) == NULL) {
    fprintf(stderr, "LoadConfig: Error.\n");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  hostname = GetValue(dic, "hostname", "localhost");
  port = atoi(GetValue(dic, "port", "8081"));

  // regex pattern for finding chat from server log
  const char *chatptn = "^.*Chat.*'([^']+)': (.*)$";
  regex_t chatreg;
  if (regcomp(&chatreg, chatptn, REG_EXTENDED | REG_NEWLINE) != 0) {
    fprintf(stderr, "regcomp: failed\n");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  // be daemon
  if (daemon(0, 0) != 0) {
    perror("daemon");
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  // make command line
  char cmd[BUFSIZ] = { 0 };
  snprintf(cmd, BUFSIZ, "telnet \"%s\" %u >\"%s\"", hostname, port, fifoname);
  // connect server
  FILE *sv;
  if ((sv = popen(cmd, "w")) == NULL) {
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }
  setbuf(sv, NULL);

  // open server output
  FILE *pp;
  if ((pp = fopen(fifoname, "r")) == NULL) {
    exit(EXIT_FAILURE);
    /* NOTREACHED */
  }

  // main loop
  char buf[BUFSIZ] = { 0 };
  char *p;
  regmatch_t match[3];
  fprintf(sv, "say \"7 Days to Die Telepotation MOD is available.\"\n");
  while (fgets(buf, BUFSIZ, pp) != NULL) {
    if (regexec(&chatreg, buf, sizeof(match) / sizeof(match[0]), match, 0) == 0) {
      char *name = buf + match[1].rm_so;
      name[match[1].rm_eo - match[1].rm_so] = 0;
      char *msg = buf + match[2].rm_so;
      *(strchr(msg, '\n')) = 0;

      if (strstr(msg, "$$ ") == msg) {
        char *key = msg + 3;
        char *arg = "";
        strchr(key, ' ') && (*(arg = strchr(key, ' ')) = 0);
        arg++;

        char temp[1024] = { 0 };
        snprintf(temp, 1024, "say \"Invalid Command -- %s\"", key);
        temp[1023] = 0;
        const char *cmd = GetValue(dic, key, temp);

        fprintf(sv, cmd, name, arg);
        fprintf(sv, "\n");
      }
    }
  }
  pclose(sv);
  fclose(pp);

  return 0;
}

///////////////////////////////////////////////////////////////////////////////

static void
TermHandler(int sig)
{
  exit(EXIT_SUCCESS);
}

static void
RemoveFIFO(void)
{
  unlink(fifoname);
}

static int
ParseConfigFile(const char *configfile)
{
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

static void
DisplayVersion(void)
{
  printf(
      "7 Days to Die Teleportation MOD  Version 0.0.0\n"
      "\tCopyright (c) 2020 KusaReMKN.  "
      "For details of license, See <https://github.com/KusaReMKN/7dtdtelemod>.\n\n"
  );
}

static void
DisplayHelp(const char *cmd)
{
  printf(
      "%1$s -- 7 Days to Die Teleportation MOD\n\n"
      "syntax:\n"
      "    %1$s -c <filename> [option]\n\n"
      "options:\n"
      "  -c, --configfile=<filename>   Specify the configuration file\n"
      "  -h, --help                    Display this help message and terminate\n"
      "  -v, --version                 Display version information and terminate\n"
    , cmd
  );
}
