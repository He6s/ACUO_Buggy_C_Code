#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

void *xmalloc(size_t size) {
    void *ptr = malloc(size == 0 ? 1 : size);
    if (ptr == NULL) {
        die("out of memory allocating %zu bytes", size);
    }
    return ptr;
}

void *xcalloc(size_t count, size_t size) {
    void *ptr = calloc(count == 0 ? 1 : count, size == 0 ? 1 : size);
    if (ptr == NULL) {
        die("out of memory allocating %zu x %zu bytes", count, size);
    }
    return ptr;
}

char *xstrdup(const char *s) {
    size_t n = strlen(s);
    char *copy = xmalloc(n + 1);
    memcpy(copy, s, n + 1);
    return copy;
}

char *read_entire_file(const char *path, size_t *out_len) {
    FILE *fp = fopen(path, "rb");
    long end;
    char *buf;
    size_t got;

    if (fp == NULL) {
        die("cannot open %s: %s", path, strerror(errno));
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        die("cannot seek %s", path);
    }
    end = ftell(fp);
    if (end < 0) {
        die("cannot tell %s", path);
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        die("cannot rewind %s", path);
    }
    buf = xmalloc((size_t)end + 1);
    got = fread(buf, 1, (size_t)end, fp);
    if (got != (size_t)end) {
        die("short read from %s", path);
    }
    buf[got] = '\0';
    if (fclose(fp) != 0) {
        die("cannot close %s", path);
    }
    *out_len = got;
    return buf;
}

char *trim_left(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }
    return s;
}

void trim_right(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

bool parse_u32(const char *text, uint32_t *out) {
    char *end = NULL;
    unsigned long value;

    if (text == NULL || *text == '\0') {
        return false;
    }
    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value > UINT32_MAX) {
        return false;
    }
    *out = (uint32_t)value;
    return true;
}

bool parse_size(const char *text, size_t *out) {
    char *end = NULL;
    unsigned long long value;

    if (text == NULL || *text == '\0') {
        return false;
    }
    errno = 0;
    value = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return false;
    }
    *out = (size_t)value;
    return true;
}

void dump_bytes(FILE *out, const void *data, size_t len) {
    const unsigned char *p = data;
    size_t i;

    for (i = 0; i < len; i++) {
        fprintf(out, "%02x", p[i]);
        if ((i + 1) % 16 == 0 || i + 1 == len) {
            fputc('\n', out);
        } else {
            fputc(' ', out);
        }
    }
}
