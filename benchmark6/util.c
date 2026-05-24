#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *message)
{
    fprintf(stderr, "fatal: %s\n", message);
    exit(1);
}

void *xcalloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    if (ptr == NULL) {
        die("out of memory");
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
    void *next = realloc(ptr, size);
    if (next == NULL) {
        die("out of memory");
    }
    return next;
}

char *xstrdup(const char *text)
{
    size_t len = strlen(text);
    char *copy = xcalloc(len + 1U, sizeof(char));
    memcpy(copy, text, len + 1U);
    return copy;
}

char *trim_left(char *text)
{
    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }
    return text;
}

void trim_right(char *text)
{
    size_t len = strlen(text);
    while (len > 0U && isspace((unsigned char)text[len - 1U])) {
        text[len - 1U] = '\0';
        len--;
    }
}

bool split_token(char **cursor, char *out, size_t out_size)
{
    char *p = *cursor;
    size_t len = 0U;

    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '\0') {
        return false;
    }
    while (p[len] != '\0' && !isspace((unsigned char)p[len])) {
        len++;
    }
    if (len + 1U > out_size) {
        return false;
    }
    memcpy(out, p, len);
    out[len] = '\0';
    p += len;
    *cursor = p;
    return true;
}

bool parse_uint(const char *text, unsigned int *out)
{
    char *end = NULL;
    unsigned long value;

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return false;
    }
    if (value > 4294967295UL) {
        return false;
    }
    *out = (unsigned int)value;
    return true;
}

bool starts_with(const char *text, const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    return strncmp(text, prefix, prefix_len) == 0;
}

void copy_text(char *dst, size_t dst_size, const char *src)
{
    size_t len;
    if (dst_size == 0U) {
        return;
    }
    len = strlen(src);
    if (len >= dst_size) {
        len = dst_size - 1U;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}
