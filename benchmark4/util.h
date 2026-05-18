#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ROUTE_NAME_MAX 31u
#define NODE_NAME_MAX 31u
#define MODE_NAME_MAX 11u
#define LINE_MAX_LEN 256u
#define NO_PARENT UINT32_MAX

typedef enum Status {
    STATUS_OK = 0,
    STATUS_ERR = 1
} Status;

typedef struct LineList {
    char **items;
    size_t count;
    size_t cap;
} LineList;

void die(const char *message);
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *text);
char *trim_newline(char *text);
int starts_with(const char *text, const char *prefix);
int parse_u32_field(const char *line, const char *key, uint32_t *out);
int parse_word_field(const char *line, const char *key, char *out, size_t out_cap);
void line_list_init(LineList *list);
void line_list_push(LineList *list, const char *line);
void line_list_free(LineList *list);
Status read_lines(const char *path, LineList *out);
uint32_t simple_checksum(const char *text);
void trace_hex(const char *label, const void *data, size_t len);

#endif
