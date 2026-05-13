#include "textbuf.h"

#include "util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t grow_capacity(size_t current, size_t required)
{
    size_t cap = current == 0U ? TEXTBUF_INITIAL_CAP : current;

    while (cap < required) {
        if (cap > SIZE_MAX / 2U) {
            cap = required;
            break;
        }
        cap *= 2U;
    }
    return cap;
}

static void textbuf_reserve(TextBuf *buf, size_t required)
{
    size_t next_cap;
    uintptr_t old_addr;
    char *next;

    if (required <= buf->cap) {
        return;
    }

    next_cap = grow_capacity(buf->cap, required);
    old_addr = (uintptr_t)buf->data;
    next = realloc(buf->data, next_cap);
    if (next == NULL) {
        die("text buffer could not grow to %zu bytes", next_cap);
    }

    buf->data = next;
    buf->cap = next_cap;
    buf->growths++;

    if (buf->trace) {
        printf("trace: text buffer realloc growth=%u old=%p new=%p cap=%zu\n",
               buf->growths,
               (void *)old_addr,
               (void *)buf->data,
               buf->cap);
    }
}

void textbuf_init(TextBuf *buf, bool trace)
{
    buf->data = NULL;
    buf->len = 0U;
    buf->cap = 0U;
    buf->growths = 0U;
    buf->trace = trace;
    textbuf_reserve(buf, TEXTBUF_INITIAL_CAP);
    if (buf->trace) {
        printf("trace: text buffer initial data=%p cap=%zu\n", (void *)buf->data, buf->cap);
    }
}

void textbuf_destroy(TextBuf *buf)
{
    free(buf->data);
    buf->data = NULL;
    buf->len = 0U;
    buf->cap = 0U;
    buf->growths = 0U;
}

TextSlice textbuf_append_cstr(TextBuf *buf, const char *text)
{
    size_t len = strlen(text);
    size_t off = buf->len;
    TextSlice slice;

    textbuf_reserve(buf, buf->len + len + 1U);
    memcpy(buf->data + buf->len, text, len + 1U);
    buf->len += len + 1U;

    slice.ptr = buf->data + off;
    slice.off = off;
    slice.len = len;

    if (buf->trace) {
        printf("trace: append string off=%zu len=%zu ptr=%p\n", off, len, (void *)slice.ptr);
    }
    return slice;
}

TextSlice textbuf_append_value(TextBuf *buf, const char *text)
{
    TextSlice slice = textbuf_append_cstr(buf, text);
    if (buf->trace) {
        print_bytes_preview("trace: value", slice.ptr, slice.len);
    }
    return slice;
}

void textbuf_append_fill(TextBuf *buf, size_t count, char fill)
{
    size_t off = buf->len;

    textbuf_reserve(buf, buf->len + count + 1U);
    memset(buf->data + buf->len, (unsigned char)fill, count);
    buf->len += count;
    buf->data[buf->len] = '\0';
    buf->len++;

    if (buf->trace) {
        printf("trace: fill off=%zu count=%zu fill='%c' current_data=%p\n",
               off,
               count,
               fill,
               (void *)buf->data);
    }
}

const char *textbuf_at(const TextBuf *buf, size_t off, size_t len)
{
    if (off > buf->len || len > buf->len - off) {
        die("text buffer offset out of range off=%zu len=%zu used=%zu", off, len, buf->len);
    }
    return buf->data + off;
}

bool textbuf_owns_ptr(const TextBuf *buf, const char *ptr)
{
    const char *start = buf->data;
    const char *end = buf->data + buf->cap;
    return ptr >= start && ptr < end;
}

void textbuf_stats(const TextBuf *buf)
{
    printf("textbuf: used=%zu cap=%zu data=%p growths=%u\n",
           buf->len,
           buf->cap,
           (void *)buf->data,
           buf->growths);
}
