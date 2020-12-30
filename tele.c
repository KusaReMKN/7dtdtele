#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define LINEMAX 1024

int main(void)
{
  char buf[LINEMAX] = { 0 };
  int i = 0;
  FILE *pp = NULL;
  FILE *sv = NULL;
  char *p = NULL;

  // telnet に書き込む．出力は named fifo に書き込む
  if ((sv = popen("telnet localhost 8081 > ./teleporter", "w")) == NULL) {
    perror("popen");
    return 2;
  }
  setbuf(sv, NULL);

  // !!! この時点で named fifo はロックされている

  // named fifo を読み込む
  if ((pp = fopen("./teleporter", "r")) == NULL) {
    perror("./teleporter");
    return 1;
  }

  fprintf(sv, "say \"Mikan 7DTD Telepoter is available.\"\n");

  while (fgets(buf, LINEMAX, pp) != NULL) {
    fputs(buf, stderr);
    usleep(10000);
    if ((p = strstr(buf, "$$ ")) != NULL) {
      p += 3;
      fprintf(stderr, "COMMAND!!\n");
      fprintf(sv, "say \"FOUND W-DOLLAR COMMAND\"\n");
      fprintf(sv, "%s", p);
    }
  }
  pclose(sv);
  fclose(pp);

  return 0;
}