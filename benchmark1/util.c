#define _POSIX_C_SOURCE 200809L
#include "util.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct PageStringHeader {
    uint64_t magic;
    size_t mapping_len;
    size_t string_len;
} PageStringHeader;

static const uint64_t PAGE_STRING_MAGIC = 0x5041474553545255ULL;

static PageStringHeader *header_from_ptr(const void *ptr) {
    if (ptr == NULL) {
        return NULL;
    }
    return ((PageStringHeader *)ptr) - 1;
}

void fatal(const char *fmt, ...) {
    va_list args;

    fprintf(stderr, "fatal: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

void *xcalloc(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    if (ptr == NULL) {
        fatal("out of memory allocating %zu bytes", count * size);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    void *resized = realloc(ptr, size);
    if (resized == NULL) {
        fatal("out of memory reallocating to %zu bytes", size);
    }
    return resized;
}

char *page_strdup_len(const char *src, size_t len) {
    long page_size = sysconf(_SC_PAGESIZE);
    size_t total;
    size_t rounded;
    PageStringHeader *hdr;
    char *dst;

    if (src == NULL) {
        return NULL;
    }
    if (page_size <= 0) {
        fatal("sysconf(_SC_PAGESIZE) failed");
    }

    total = sizeof(PageStringHeader) + len + 1;
    rounded = (size_t)(((total + (size_t)page_size - 1U) / (size_t)page_size) * (size_t)page_size);

    hdr = mmap(NULL, rounded, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (hdr == MAP_FAILED) {
        fatal("mmap failed while allocating %zu bytes", rounded);
    }

    hdr->magic = PAGE_STRING_MAGIC;
    hdr->mapping_len = rounded;
    hdr->string_len = len;

    dst = (char *)(hdr + 1);
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

char *page_strdup(const char *src) {
    return page_strdup_len(src, strlen(src));
}


void page_str_poison(char *ptr) {
    PageStringHeader *hdr;

    if (ptr == NULL) {
        return;
    }

    hdr = header_from_ptr(ptr);
    if (hdr->magic != PAGE_STRING_MAGIC) {
        fatal("attempted to poison a non-page string at %p", (void *)ptr);
    }

    if (mprotect(hdr, hdr->mapping_len, PROT_NONE) != 0) {
        fatal("mprotect(PROT_NONE) failed for %p", (void *)ptr);
    }
}
void page_strfree(char *ptr) {
    PageStringHeader *hdr;

    if (ptr == NULL) {
        return;
    }

    hdr = header_from_ptr(ptr);
    if (hdr->magic != PAGE_STRING_MAGIC) {
        fatal("attempted to free a non-page string at %p", (void *)ptr);
    }

    if (munmap(hdr, hdr->mapping_len) != 0) {
        fatal("munmap failed while freeing %p", (void *)ptr);
    }
}

size_t page_str_len(const char *ptr) {
    PageStringHeader *hdr;

    if (ptr == NULL) {
        return 0;
    }
    hdr = header_from_ptr(ptr);
    if (hdr->magic != PAGE_STRING_MAGIC) {
        fatal("page_str_len called on non-page string %p", (const void *)ptr);
    }
    return hdr->string_len;
}

bool page_ptr_owned(const void *ptr) {
    const PageStringHeader *hdr;

    if (ptr == NULL) {
        return false;
    }

    hdr = header_from_ptr(ptr);
    return hdr->magic == PAGE_STRING_MAGIC;
}

void trim_newline(char *s) {
    size_t len;

    if (s == NULL) {
        return;
    }
    len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

char *skip_ws(char *s) {
    while (s != NULL && *s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }
    return s;
}

char *next_token(char **cursor) {
    char *start;
    char *end;

    if (cursor == NULL || *cursor == NULL) {
        return NULL;
    }

    start = skip_ws(*cursor);
    if (*start == '\0') {
        *cursor = start;
        return NULL;
    }

    end = start;
    while (*end != '\0' && !isspace((unsigned char)*end)) {
        end++;
    }

    if (*end != '\0') {
        *end = '\0';
        end++;
    }

    *cursor = end;
    return start;
}

void lowercase_ascii(char *s) {
    for (; s != NULL && *s != '\0'; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

void debug_dump_ptr(FILE *out, const char *label, const void *ptr) {
    fprintf(out, "%s=%p", label, ptr);
    if (ptr != NULL && page_ptr_owned(ptr)) {
        fprintf(out, " len=%zu", page_str_len(ptr));
    }
    fputc('\n', out);
}

uint64_t fnv1a_bytes(const void *data, size_t len) {
    const unsigned char *bytes = data;
    uint64_t hash = 1469598103934665603ULL;
    size_t i;

    for (i = 0; i < len; ++i) {
        hash ^= (uint64_t)bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}
