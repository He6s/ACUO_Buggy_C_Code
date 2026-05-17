#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Cursor {
    const unsigned char *base;
    const unsigned char *p;
    const unsigned char *end;
    int trace;
} Cursor;

typedef struct DecodedField {
    unsigned char *data;
    size_t len;
    size_t declared_len;
    size_t raw_offset;
    size_t next_offset;
} DecodedField;

static size_t cursor_offset(const Cursor *cur)
{
    return (size_t)(cur->p - cur->base);
}

static int cursor_has(const Cursor *cur, const char *literal)
{
    return util_starts_with(cur->p, cur->end, literal);
}

static int cursor_expect(Cursor *cur, const char *literal)
{
    size_t n = strlen(literal);
    if (!cursor_has(cur, literal)) {
        fprintf(stderr, "parse error at offset %zu: expected '%s'\n",
                cursor_offset(cur), literal);
        return B3_ERR;
    }
    cur->p += n;
    return B3_OK;
}

static void skip_to_next_record(Cursor *cur)
{
    while (cur->p < cur->end) {
        if (cursor_has(cur, "NODE ")) {
            return;
        }
        cur->p++;
    }
}

static void consume_header(Cursor *cur)
{
    if (cursor_has(cur, "B3FRAME 1")) {
        while (cur->p < cur->end && *cur->p != '\n') {
            cur->p++;
        }
        if (cur->p < cur->end) {
            cur->p++;
        }
    }
}

static int parse_uint_after(Cursor *cur, const char *name, unsigned int *out)
{
    const unsigned char *next;
    if (cursor_expect(cur, name) != B3_OK) {
        return B3_ERR;
    }
    if (util_parse_uint(cur->p, cur->end, out, &next) != B3_OK) {
        fprintf(stderr, "parse error at offset %zu: bad integer after %s\n",
                cursor_offset(cur), name);
        return B3_ERR;
    }
    cur->p = next;
    return B3_OK;
}

static int hex_value(unsigned char c, unsigned char *out)
{
    if (c >= '0' && c <= '9') {
        *out = (unsigned char)(c - '0');
        return B3_OK;
    }
    if (c >= 'a' && c <= 'f') {
        *out = (unsigned char)(10 + c - 'a');
        return B3_OK;
    }
    if (c >= 'A' && c <= 'F') {
        *out = (unsigned char)(10 + c - 'A');
        return B3_OK;
    }
    return B3_ERR;
}

static int decode_percent_span(const unsigned char *raw,
                               size_t raw_len,
                               unsigned char *out,
                               size_t *out_len)
{
    size_t i = 0;
    size_t j = 0;

    while (i < raw_len) {
        if (raw[i] == '%' && i + 2 < raw_len) {
            unsigned char hi;
            unsigned char lo;
            if (hex_value(raw[i + 1], &hi) == B3_OK &&
                hex_value(raw[i + 2], &lo) == B3_OK) {
                out[j++] = (unsigned char)((hi << 4) | lo);
                i += 3;
                continue;
            }
        }
        out[j++] = raw[i++];
    }

    *out_len = j;
    return B3_OK;
}

static int parse_raw_field(Cursor *cur, size_t declared_len, char **out)
{
    const unsigned char *raw = cur->p;
    if ((size_t)(cur->end - cur->p) < declared_len) {
        fprintf(stderr, "parse error at offset %zu: short raw field\n",
                cursor_offset(cur));
        return B3_ERR;
    }
    *out = util_xstrndup(raw, declared_len);
    cur->p += declared_len;
    return B3_OK;
}

static int parse_decoded_data_field(Cursor *cur,
                                    size_t declared_len,
                                    DecodedField *out)
{
    const unsigned char *raw = cur->p;
    size_t decoded_len = 0;
    unsigned char *decoded;

    if ((size_t)(cur->end - cur->p) < declared_len) {
        fprintf(stderr, "parse error at offset %zu: short data field\n",
                cursor_offset(cur));
        return B3_ERR;
    }

    decoded = util_xmalloc(declared_len + 1);
    if (decode_percent_span(raw, declared_len, decoded, &decoded_len) != B3_OK) {
        free(decoded);
        return B3_ERR;
    }
    decoded[decoded_len] = '\0';

    out->data = decoded;
    out->len = decoded_len;
    out->declared_len = declared_len;
    out->raw_offset = (size_t)(raw - cur->base);
    out->next_offset = out->raw_offset + decoded_len;

    cur->p = raw + decoded_len;
    return B3_OK;
}

static void parsed_doc_push(ParsedDoc *doc, ParsedNode node)
{
    if (doc->count == doc->capacity) {
        size_t next_cap = doc->capacity == 0 ? 8 : doc->capacity * 2;
        doc->nodes = util_xrealloc(doc->nodes, next_cap * sizeof(doc->nodes[0]));
        doc->capacity = next_cap;
    }
    doc->nodes[doc->count++] = node;
}

static int parse_node(Cursor *cur, ParsedNode *node)
{
    unsigned int key_len_u;
    unsigned int data_len_u;
    DecodedField data;
    size_t start = cursor_offset(cur);

    memset(node, 0, sizeof(*node));
    memset(&data, 0, sizeof(data));

    if (cursor_expect(cur, "NODE ") != B3_OK) {
        return B3_ERR;
    }
    if (parse_uint_after(cur, "id=", &node->id) != B3_OK) {
        return B3_ERR;
    }
    if (cursor_expect(cur, " ") != B3_OK) {
        return B3_ERR;
    }
    if (parse_uint_after(cur, "parent=", &node->parent_id) != B3_OK) {
        return B3_ERR;
    }
    if (cursor_expect(cur, " ") != B3_OK) {
        return B3_ERR;
    }
    if (parse_uint_after(cur, "keylen=", &key_len_u) != B3_OK) {
        return B3_ERR;
    }
    if (cursor_expect(cur, " key=") != B3_OK) {
        return B3_ERR;
    }
    if (parse_raw_field(cur, key_len_u, &node->key) != B3_OK) {
        return B3_ERR;
    }
    node->key_len = key_len_u;
    if (cursor_expect(cur, " datalen=") != B3_OK) {
        return B3_ERR;
    }
    if (util_parse_uint(cur->p, cur->end, &data_len_u, &cur->p) != B3_OK) {
        fprintf(stderr, "parse error at offset %zu: bad datalen\n", cursor_offset(cur));
        return B3_ERR;
    }
    if (cursor_expect(cur, " data=") != B3_OK) {
        return B3_ERR;
    }
    if (parse_decoded_data_field(cur, data_len_u, &data) != B3_OK) {
        return B3_ERR;
    }

    node->payload = data.data;
    node->payload_len = data.len;
    node->declared_payload_len = data.declared_len;
    node->raw_offset = start;
    node->next_offset = cursor_offset(cur);

    if (cur->trace) {
        fprintf(stderr,
                "trace: parsed node id=%u parent=%u key=%s start=%zu data_raw=%zu declared=%zu decoded=%zu next=%zu\n",
                node->id,
                node->parent_id,
                node->key,
                start,
                data.raw_offset,
                data.declared_len,
                data.len,
                node->next_offset);
        util_trace_bytes(stderr, "trace: decoded payload",
                         node->payload, node->payload_len, 48);
        if (cur->p < cur->end) {
            util_trace_bytes(stderr, "trace: next raw bytes",
                             cur->p, (size_t)(cur->end - cur->p), 48);
        }
    }

    return B3_OK;
}

void parser_init_doc(ParsedDoc *doc)
{
    doc->nodes = NULL;
    doc->count = 0;
    doc->capacity = 0;
}

void parser_free_doc(ParsedDoc *doc)
{
    size_t i;
    for (i = 0; i < doc->count; i++) {
        free(doc->nodes[i].key);
        free(doc->nodes[i].payload);
    }
    free(doc->nodes);
    doc->nodes = NULL;
    doc->count = 0;
    doc->capacity = 0;
}

int parser_parse_document(const ByteBuffer *buf, ParsedDoc *out, int trace)
{
    Cursor cur;

    cur.base = buf->data;
    cur.p = buf->data;
    cur.end = buf->data + buf->len;
    cur.trace = trace;

    consume_header(&cur);

    while (cur.p < cur.end) {
        ParsedNode node;
        skip_to_next_record(&cur);
        if (cur.p >= cur.end) {
            break;
        }
        if (parse_node(&cur, &node) != B3_OK) {
            return B3_ERR;
        }
        parsed_doc_push(out, node);
    }

    if (trace) {
        fprintf(stderr, "trace: parser produced %zu nodes\n", out->count);
    }
    return B3_OK;
}
