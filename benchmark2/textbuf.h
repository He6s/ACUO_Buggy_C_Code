#ifndef TEXTBUF_H
#define TEXTBUF_H

#include <stdbool.h>
#include <stddef.h>

#define TEXTBUF_INITIAL_CAP 196608U

typedef struct TextSlice {
    char *ptr;
    size_t off;
    size_t len;
} TextSlice;

typedef struct TextBuf {
    char *data;
    size_t len;
    size_t cap;
    unsigned growths;
    bool trace;
} TextBuf;

void textbuf_init(TextBuf *buf, bool trace);
void textbuf_destroy(TextBuf *buf);
TextSlice textbuf_append_cstr(TextBuf *buf, const char *text);
TextSlice textbuf_append_value(TextBuf *buf, const char *text);
void textbuf_append_fill(TextBuf *buf, size_t count, char fill);
const char *textbuf_at(const TextBuf *buf, size_t off, size_t len);
bool textbuf_owns_ptr(const TextBuf *buf, const char *ptr);
void textbuf_stats(const TextBuf *buf);

#endif
