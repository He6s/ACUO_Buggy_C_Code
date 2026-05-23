#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdio.h>

void die(const char *fmt, ...);
void *xcalloc(size_t count, size_t size);
char *xstrdup(const char *s);
void trim_line(char *s);
int starts_with(const char *s, const char *prefix);
int parse_u32(const char *text, unsigned int *out);
void tracef(int enabled, const char *fmt, ...);

#endif
