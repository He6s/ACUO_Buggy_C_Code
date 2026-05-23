#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
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

void *xcalloc(size_t count, size_t size) {
    void *p = calloc(count, size);
    if (p == NULL) {
        die("allocation failed for %zu objects of size %zu", count, size);
    }
    return p;
}

char *xstrdup(const char *s) {
    size_t len = strlen(s) + 1U;
    char *copy = xcalloc(len, 1U);
    memcpy(copy, s, len);
    return copy;
}

void trim_line(char *s) {
    size_t len = strlen(s);
    while (len > 0U && (s[len - 1U] == '\n' || s[len - 1U] == '\r' || isspace((unsigned char)s[len - 1U]))) {
        s[len - 1U] = '\0';
        len--;
    }

    char *start = s;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != s) {
        memmove(s, start, strlen(start) + 1U);
    }
}

int starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

int parse_u32(const char *text, unsigned int *out) {
    char *end = NULL;
    errno = 0;
    unsigned long value = strtoul(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || value > UINT32_MAX) {
        return 0;
    }
    *out = (unsigned int)value;
    return 1;
}

void tracef(int enabled, const char *fmt, ...) {
    if (!enabled) {
        return;
    }
    fputs("[trace] ", stdout);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fputc('\n', stdout);
}
