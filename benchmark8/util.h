#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void die(const char *fmt, ...);
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
char *xstrdup(const char *s);
char *read_entire_file(const char *path, size_t *out_len);
char *trim_left(char *s);
void trim_right(char *s);
bool parse_u32(const char *text, uint32_t *out);
bool parse_size(const char *text, size_t *out);
void dump_bytes(FILE *out, const void *data, size_t len);

#endif
