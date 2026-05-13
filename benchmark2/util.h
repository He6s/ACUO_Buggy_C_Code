#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PROGRAM_NAME "sliceidx"
#define LINE_LIMIT 4096U

void die(const char *fmt, ...);
void *xmalloc(size_t nbytes);
void *xcalloc(size_t count, size_t nbytes);
void *xrealloc(void *ptr, size_t nbytes);
char *xstrdup(const char *text);

bool parse_size_arg(const char *text, size_t *out);
bool is_blank_or_comment(const char *line);
void strip_newline(char *line);
void trim_left_in_place(char **text);
void trim_right_in_place(char *text);
bool safe_copy(char *dst, size_t dst_cap, const char *src);

bool read_line(FILE *fp, char **buf, size_t *cap, size_t *len_out);
uint64_t hash_bytes(const void *data, size_t len);
void print_bytes_preview(const char *label, const char *data, size_t len);

#endif
