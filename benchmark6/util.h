#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_LINE 512U
#define MAX_NAME 64U
#define MAX_TEXT 192U

void die(const char *message);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *text);
char *trim_left(char *text);
void trim_right(char *text);
bool split_token(char **cursor, char *out, size_t out_size);
bool parse_uint(const char *text, unsigned int *out);
bool starts_with(const char *text, const char *prefix);
void copy_text(char *dst, size_t dst_size, const char *src);

#endif
