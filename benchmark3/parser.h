#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "util.h"

typedef struct ParsedNode {
    unsigned int id;
    unsigned int parent_id;
    char *key;
    unsigned char *payload;
    size_t key_len;
    size_t payload_len;
    size_t declared_payload_len;
    size_t raw_offset;
    size_t next_offset;
} ParsedNode;

typedef struct ParsedDoc {
    ParsedNode *nodes;
    size_t count;
    size_t capacity;
} ParsedDoc;

void parser_init_doc(ParsedDoc *doc);
void parser_free_doc(ParsedDoc *doc);
int parser_parse_document(const ByteBuffer *buf, ParsedDoc *out, int trace);

#endif
