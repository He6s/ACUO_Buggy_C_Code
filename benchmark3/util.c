#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *util_xmalloc(size_t size)
{
    void *p = malloc(size == 0 ? 1 : size);
    if (p == NULL) {
        fprintf(stderr, "out of memory allocating %zu bytes\n", size);
        exit(2);
    }
    return p;
}

void *util_xcalloc(size_t count, size_t size)
{
    void *p = calloc(count == 0 ? 1 : count, size == 0 ? 1 : size);
    if (p == NULL) {
        fprintf(stderr, "out of memory allocating %zu x %zu bytes\n", count, size);
        exit(2);
    }
    return p;
}

void *util_xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size == 0 ? 1 : size);
    if (p == NULL) {
        fprintf(stderr, "out of memory resizing to %zu bytes\n", size);
        exit(2);
    }
    return p;
}

char *util_xstrndup(const unsigned char *src, size_t len)
{
    char *copy = util_xmalloc(len + 1);
    if (len > 0) {
        memcpy(copy, src, len);
    }
    copy[len] = '\0';
    return copy;
}

int util_load_file(const char *path, ByteBuffer *out)
{
    FILE *fp;
    long size;
    size_t got;

    out->data = NULL;
    out->len = 0;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "cannot open %s: %s\n", path, strerror(errno));
        return B3_ERR;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "cannot seek %s\n", path);
        fclose(fp);
        return B3_ERR;
    }

    size = ftell(fp);
    if (size < 0) {
        fprintf(stderr, "cannot size %s\n", path);
        fclose(fp);
        return B3_ERR;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fprintf(stderr, "cannot rewind %s\n", path);
        fclose(fp);
        return B3_ERR;
    }

    out->data = util_xmalloc((size_t)size + 1);
    got = fread(out->data, 1, (size_t)size, fp);
    if (got != (size_t)size) {
        fprintf(stderr, "short read from %s\n", path);
        fclose(fp);
        util_free_buffer(out);
        return B3_ERR;
    }
    out->data[got] = '\0';
    out->len = got;
    fclose(fp);
    return B3_OK;
}

void util_free_buffer(ByteBuffer *buf)
{
    free(buf->data);
    buf->data = NULL;
    buf->len = 0;
}

int util_parse_uint(const unsigned char *begin,
                    const unsigned char *end,
                    unsigned int *out,
                    const unsigned char **next)
{
    unsigned long value = 0;
    const unsigned char *p = begin;

    if (p >= end || !isdigit((unsigned char)*p)) {
        return B3_ERR;
    }

    while (p < end && isdigit((unsigned char)*p)) {
        value = value * 10UL + (unsigned long)(*p - '0');
        if (value > UINT32_MAX) {
            return B3_ERR;
        }
        p++;
    }

    *out = (unsigned int)value;
    *next = p;
    return B3_OK;
}

int util_starts_with(const unsigned char *p,
                     const unsigned char *end,
                     const char *literal)
{
    size_t n = strlen(literal);
    if ((size_t)(end - p) < n) {
        return 0;
    }
    return memcmp(p, literal, n) == 0;
}

void util_trace_bytes(FILE *fp,
                      const char *label,
                      const unsigned char *data,
                      size_t len,
                      size_t max_len)
{
    size_t shown = len < max_len ? len : max_len;
    size_t i;

    fprintf(fp, "%s len=%zu bytes=", label, len);
    for (i = 0; i < shown; i++) {
        unsigned char c = data[i];
        if (c >= 32 && c <= 126 && c != '\\') {
            fputc((int)c, fp);
        } else if (c == '\n') {
            fputs("\\n", fp);
        } else if (c == '\r') {
            fputs("\\r", fp);
        } else if (c == '\\') {
            fputs("\\\\", fp);
        } else {
            fprintf(fp, "\\x%02x", c);
        }
    }
    if (shown < len) {
        fputs("...", fp);
    }
    fputc('\n', fp);
}

unsigned long util_hash_bytes(const unsigned char *data, size_t len)
{
    unsigned long h = 1469598103934665603UL;
    size_t i;

    for (i = 0; i < len; i++) {
        h ^= (unsigned long)data[i];
        h *= 1099511628211UL;
    }
    return h;
}
