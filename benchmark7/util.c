#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tracef(const Trace *trace, const char *fmt, ...) {
    if (trace == NULL || !trace->enabled) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    fputs("[trace] ", stdout);
    vfprintf(stdout, fmt, ap);
    fputc('\n', stdout);
    fflush(stdout);
    va_end(ap);
}

void fatal(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("error: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

static long file_size(FILE *fp) {
    if (fseek(fp, 0, SEEK_END) != 0) {
        fatal("failed to seek input file");
    }
    long size = ftell(fp);
    if (size < 0) {
        fatal("failed to tell input file size");
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fatal("failed to rewind input file");
    }
    return size;
}

char *read_text_file(const char *path, size_t *out_len) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fatal("unable to open %s", path);
    }

    long sz = file_size(fp);
    char *buf = (char *)malloc((size_t)sz + 1U);
    if (buf == NULL) {
        fclose(fp);
        fatal("out of memory while reading %s", path);
    }

    size_t nread = fread(buf, 1U, (size_t)sz, fp);
    if (nread != (size_t)sz) {
        free(buf);
        fclose(fp);
        fatal("short read while reading %s", path);
    }
    buf[nread] = '\0';
    fclose(fp);

    if (out_len != NULL) {
        *out_len = nread;
    }
    return buf;
}

char *trim_left(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) {
        ++s;
    }
    return s;
}

void trim_right(char *s) {
    size_t len = strlen(s);
    while (len > 0U && isspace((unsigned char)s[len - 1U])) {
        s[len - 1U] = '\0';
        --len;
    }
}

int split_key_value(char *line, char **key, char **value) {
    char *p = trim_left(line);
    trim_right(p);

    if (*p == '\0' || *p == '#') {
        return 0;
    }

    char *space = p;
    while (*space != '\0' && !isspace((unsigned char)*space)) {
        ++space;
    }

    if (*space == '\0') {
        fatal("line without value: %s", p);
    }

    *space = '\0';
    ++space;
    space = trim_left(space);
    trim_right(space);

    if (*space == '\0') {
        fatal("empty value for %s", p);
    }

    *key = p;
    *value = space;
    return 1;
}

uint32_t parse_u32_strict(const char *text, const char *field) {
    errno = 0;
    char *end = NULL;
    unsigned long v = strtoul(text, &end, 10);
    if (text == end || errno != 0 || *end != '\0' || v > UINT32_MAX) {
        fatal("invalid integer for %s: %s", field, text);
    }
    return (uint32_t)v;
}

uint32_t checked_mul_u32(uint32_t a, uint32_t b, const char *what) {
    if (a != 0U && b > UINT32_MAX / a) {
        fatal("%s overflows uint32_t", what);
    }
    return a * b;
}

uint32_t next_lcg(uint32_t *state) {
    *state = (*state * 1664525U) + 1013904223U;
    return *state;
}

char *xstrdup(const char *s) {
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1U);
    if (copy == NULL) {
        fatal("out of memory duplicating string");
    }
    memcpy(copy, s, len + 1U);
    return copy;
}
