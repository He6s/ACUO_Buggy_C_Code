#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Trace {
    int enabled;
} Trace;

void tracef(const Trace *trace, const char *fmt, ...);
void fatal(const char *fmt, ...);
char *read_text_file(const char *path, size_t *out_len);
char *trim_left(char *s);
void trim_right(char *s);
int split_key_value(char *line, char **key, char **value);
uint32_t parse_u32_strict(const char *text, const char *field);
uint32_t checked_mul_u32(uint32_t a, uint32_t b, const char *what);
uint32_t next_lcg(uint32_t *state);
char *xstrdup(const char *s);

#endif
