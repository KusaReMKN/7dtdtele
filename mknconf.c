#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

#include "mknconf.h"

static const char *StatementPattern =
"[[:space:]]*([^[:space:]]+)[[:space:]]*:[[:space:]]*(.*)[[:space:]]*";


MKNDIC *LoadConfig(const char *filename)
{
  FILE *fp;
  char buf[BUFSIZ] = { 0 };
  regex_t preg;
  regmatch_t match[3];

  MKNDIC *ret = NULL, *tmp = NULL;

  if (regcomp(&preg, StatementPattern, REG_EXTENDED | REG_NEWLINE) != 0) {
    fprintf(stderr, "regcomp\n");
    return NULL;
  }

  if ((fp = fopen(filename, "r")) == NULL) {
    return NULL;
  }

  while (fgets(buf, BUFSIZ, fp) != NULL) {
    *(strchr(buf, '\n')) = 0;

    // Comment?
    if (buf[0] == '#') continue;
    if (buf[0] == 0) continue;

    // error?
    if (regexec(&preg, buf, 3, match, 0) != 0) {
      continue;
    }

    char *key = buf + match[1].rm_so;
    char *value = buf + match[2].rm_so;
    key[match[1].rm_eo - match[1].rm_so] = 0;
    value[match[2].rm_eo - match[2].rm_so] = 0;

    tmp = ret;
    if ((ret = malloc(sizeof(MKNDIC))) == NULL) {
      return tmp;
    }
    if ((ret->key = malloc(strlen(key) + strlen(value) + 2)) == NULL) {
      return tmp;
    }
    strcpy(ret->key, key);
    ret->value = ret->key + strlen(ret->key) + 1;
    strcpy(ret->value, value);
    ret->next = tmp;
  }

  fclose(fp);
  regfree(&preg);

  return ret;
}

const char *GetValue(MKNDIC *dic, const char *key, const char *def)
{
  while (dic != NULL) {
    if (strcmp(dic->key, key) == 0) {
      return dic->value;
    }
    dic = dic->next;
  }
  return def;
}

void FreeConfig(MKNDIC *dic)
{
  while (dic != NULL) {
    MKNDIC *tmp = dic->next;
    free(dic->key);
    free(dic);
    dic = dic->next;
  }
}
