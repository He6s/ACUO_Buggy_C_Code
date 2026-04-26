#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void fatal(const char *fmt, ...);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);
char *page_strdup_len(const char *src, size_t len);
char *page_strdup(const char *src);
void page_strfree(char *ptr);
void page_str_poison(char *ptr);
size_t page_str_len(const char *ptr);
bool page_ptr_owned(const void *ptr);
void trim_newline(char *s);
char *skip_ws(char *s);
char *next_token(char **cursor);
void lowercase_ascii(char *s);
void debug_dump_ptr(FILE *out, const char *label, const void *ptr);
uint64_t fnv1a_bytes(const void *data, size_t len);

#endif
