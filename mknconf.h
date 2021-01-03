#pragma once
#ifndef MKNCONF_H
#define MKNCONF_H

typedef struct mkndic_t {
  char *key;
  char *value;
  struct mkndic_t *next;
} MKNDIC;

#ifdef __cplusplus
extern "C" {
#endif

extern MKNDIC *LoadConfig(const char *);
extern const char *GetValue(MKNDIC *, const char *, const char *);
extern void FreeConfig(MKNDIC *);

#ifdef __cplusplus
}
#endif

#endif /* MKNCONF_H */
