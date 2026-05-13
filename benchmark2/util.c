#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void die(const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "%s: ", PROGRAM_NAME);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

void *xmalloc(size_t nbytes)
{
    void *ptr = malloc(nbytes == 0U ? 1U : nbytes);
    if (ptr == NULL) {
        die("out of memory allocating %zu bytes", nbytes);
    }
    return ptr;
}

void *xcalloc(size_t count, size_t nbytes)
{
    void *ptr = calloc(count == 0U ? 1U : count, nbytes == 0U ? 1U : nbytes);
    if (ptr == NULL) {
        die("out of memory allocating %zu elements", count);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t nbytes)
{
    void *next = realloc(ptr, nbytes == 0U ? 1U : nbytes);
    if (next == NULL) {
        die("out of memory resizing to %zu bytes", nbytes);
    }
    return next;
}

char *xstrdup(const char *text)
{
    size_t len = strlen(text);
    char *copy = xmalloc(len + 1U);
    memcpy(copy, text, len + 1U);
    return copy;
}

bool parse_size_arg(const char *text, size_t *out)
{
    char *end = NULL;
    unsigned long long value;

    if (text == NULL || *text == '\0') {
        return false;
    }
    if (*text == '-') {
        return false;
    }

    errno = 0;
    value = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return false;
    }
    if ((uintmax_t)value > (uintmax_t)SIZE_MAX) {
        return false;
    }
    *out = (size_t)value;
    return true;
}

bool is_blank_or_comment(const char *line)
{
    const unsigned char *p = (const unsigned char *)line;
    while (*p != '\0' && isspace(*p)) {
        p++;
    }
    return *p == '\0' || *p == '#';
}

void strip_newline(char *line)
{
    size_t len = strlen(line);
    while (len > 0U && (line[len - 1U] == '\n' || line[len - 1U] == '\r')) {
        line[len - 1U] = '\0';
        len--;
    }
}

void trim_left_in_place(char **text)
{
    unsigned char *p = (unsigned char *)*text;
    while (*p != '\0' && isspace(*p)) {
        p++;
    }
    *text = (char *)p;
}

void trim_right_in_place(char *text)
{
    size_t len = strlen(text);
    while (len > 0U && isspace((unsigned char)text[len - 1U])) {
        text[len - 1U] = '\0';
        len--;
    }
}

bool safe_copy(char *dst, size_t dst_cap, const char *src)
{
    size_t len = strlen(src);
    if (len + 1U > dst_cap) {
        return false;
    }
    memcpy(dst, src, len + 1U);
    return true;
}

bool read_line(FILE *fp, char **buf, size_t *cap, size_t *len_out)
{
    int ch;
    size_t len = 0U;

    if (*buf == NULL || *cap == 0U) {
        *cap = 256U;
        *buf = xmalloc(*cap);
    }

    while ((ch = fgetc(fp)) != EOF) {
        if (len + 1U >= *cap) {
            if (*cap >= LINE_LIMIT) {
                die("input line exceeds %u bytes", LINE_LIMIT);
            }
            *cap *= 2U;
            if (*cap > LINE_LIMIT) {
                *cap = LINE_LIMIT + 1U;
            }
            *buf = xrealloc(*buf, *cap);
        }
        (*buf)[len] = (char)ch;
        len++;
        if (ch == '\n') {
            break;
        }
    }

    if (len == 0U && ch == EOF) {
        return false;
    }
    (*buf)[len] = '\0';
    if (len_out != NULL) {
        *len_out = len;
    }
    return true;
}

uint64_t hash_bytes(const void *data, size_t len)
{
    const unsigned char *p = data;
    uint64_t h = 1469598103934665603ULL;
    size_t i;

    for (i = 0U; i < len; i++) {
        h ^= (uint64_t)p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

void print_bytes_preview(const char *label, const char *data, size_t len)
{
    size_t shown = len < 24U ? len : 24U;
    size_t i;

    printf("%s len=%zu data=\"", label, len);
    for (i = 0U; i < shown; i++) {
        unsigned char c = (unsigned char)data[i];
        if (isprint(c)) {
            putchar((int)c);
        } else {
            printf("\\x%02x", c);
        }
    }
    if (shown < len) {
        printf("...");
    }
    printf("\"\n");
}
