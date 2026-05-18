#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *message) {
    fprintf(stderr, "fatal: %s\n", message);
    exit(1);
}

void *xmalloc(size_t size) {
    void *ptr = malloc(size == 0 ? 1 : size);
    if (ptr == NULL) {
        die("out of memory");
    }
    return ptr;
}

void *xcalloc(size_t count, size_t size) {
    void *ptr = calloc(count == 0 ? 1 : count, size == 0 ? 1 : size);
    if (ptr == NULL) {
        die("out of memory");
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    void *next = realloc(ptr, size == 0 ? 1 : size);
    if (next == NULL) {
        die("out of memory");
    }
    return next;
}

char *xstrdup(const char *text) {
    size_t len = strlen(text);
    char *copy = xmalloc(len + 1);
    memcpy(copy, text, len + 1);
    return copy;
}

char *trim_newline(char *text) {
    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
    return text;
}

int starts_with(const char *text, const char *prefix) {
    return strncmp(text, prefix, strlen(prefix)) == 0;
}

static const char *field_start(const char *line, const char *key) {
    size_t key_len = strlen(key);
    const char *cursor = line;
    while (*cursor != '\0') {
        while (*cursor == ' ') {
            cursor++;
        }
        if (strncmp(cursor, key, key_len) == 0 && cursor[key_len] == '=') {
            return cursor + key_len + 1;
        }
        while (*cursor != '\0' && *cursor != ' ') {
            cursor++;
        }
    }
    return NULL;
}

int parse_u32_field(const char *line, const char *key, uint32_t *out) {
    const char *start = field_start(line, key);
    char *end = NULL;
    unsigned long value;
    if (start == NULL || *start == '\0') {
        return 0;
    }
    errno = 0;
    value = strtoul(start, &end, 10);
    if (errno != 0 || end == start || value > UINT32_MAX) {
        return 0;
    }
    *out = (uint32_t)value;
    return 1;
}

int parse_word_field(const char *line, const char *key, char *out, size_t out_cap) {
    const char *start = field_start(line, key);
    size_t len = 0;
    if (start == NULL || out_cap == 0) {
        return 0;
    }
    while (start[len] != '\0' && !isspace((unsigned char)start[len])) {
        len++;
    }
    if (len + 1 > out_cap) {
        return 0;
    }
    memcpy(out, start, len);
    out[len] = '\0';
    return 1;
}

void line_list_init(LineList *list) {
    list->items = NULL;
    list->count = 0;
    list->cap = 0;
}

void line_list_push(LineList *list, const char *line) {
    if (list->count == list->cap) {
        size_t next_cap = list->cap == 0 ? 16 : list->cap * 2;
        list->items = xrealloc(list->items, next_cap * sizeof(list->items[0]));
        list->cap = next_cap;
    }
    list->items[list->count++] = xstrdup(line);
}

void line_list_free(LineList *list) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->cap = 0;
}

Status read_lines(const char *path, LineList *out) {
    FILE *fp;
    char buffer[LINE_MAX_LEN];
    fp = fopen(path, "r");
    if (fp == NULL) {
        return STATUS_ERR;
    }
    line_list_init(out);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        trim_newline(buffer);
        if (buffer[0] == '\0' || buffer[0] == '#') {
            continue;
        }
        line_list_push(out, buffer);
    }
    if (ferror(fp)) {
        fclose(fp);
        line_list_free(out);
        return STATUS_ERR;
    }
    fclose(fp);
    return STATUS_OK;
}

uint32_t simple_checksum(const char *text) {
    uint32_t hash = 2166136261u;
    while (*text != '\0') {
        hash ^= (unsigned char)*text;
        hash *= 16777619u;
        text++;
    }
    return hash;
}

void trace_hex(const char *label, const void *data, size_t len) {
    const unsigned char *bytes = data;
    size_t i;
    printf("trace: %s", label);
    for (i = 0; i < len; i++) {
        if (i % 16u == 0u) {
            printf("\ntrace:   %04zu:", i);
        }
        printf(" %02x", bytes[i]);
    }
    printf("\n");
}
