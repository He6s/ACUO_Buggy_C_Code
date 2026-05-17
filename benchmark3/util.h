#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdio.h>

#define B3_OK 0
#define B3_ERR 1

typedef struct ByteBuffer {
    unsigned char *data;
    size_t len;
} ByteBuffer;

int util_load_file(const char *path, ByteBuffer *out);
void util_free_buffer(ByteBuffer *buf);
void *util_xmalloc(size_t size);
void *util_xcalloc(size_t count, size_t size);
void *util_xrealloc(void *ptr, size_t size);
char *util_xstrndup(const unsigned char *src, size_t len);
int util_parse_uint(const unsigned char *begin,
                    const unsigned char *end,
                    unsigned int *out,
                    const unsigned char **next);
int util_starts_with(const unsigned char *p,
                     const unsigned char *end,
                     const char *literal);
void util_trace_bytes(FILE *fp,
                      const char *label,
                      const unsigned char *data,
                      size_t len,
                      size_t max_len);
unsigned long util_hash_bytes(const unsigned char *data, size_t len);

#endif
