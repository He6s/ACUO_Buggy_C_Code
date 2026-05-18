#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HeaderFrame {
    char label[ROUTE_NAME_MAX + 1];
    uint32_t parent_index;
    uint32_t declared_nodes;
    char mode[MODE_NAME_MAX + 1];
    uint32_t header_checksum;
} HeaderFrame;

typedef struct ParserCursor {
    const LineList *lines;
    size_t pos;
    int trace;
} ParserCursor;

static Status cursor_next(ParserCursor *cursor, const char **out) {
    if (cursor->pos >= cursor->lines->count) {
        return STATUS_ERR;
    }
    *out = cursor->lines->items[cursor->pos++];
    return STATUS_OK;
}

static void header_frame_init(HeaderFrame *frame) {
    memset(frame, 0, sizeof(*frame));
    frame->parent_index = NO_PARENT;
    frame->declared_nodes = 0;
    memcpy(frame->mode, "direct", 7);
    frame->header_checksum = 0x13572468u;
}

static uint32_t parse_parent_token(const char *route_line) {
    char value[32];
    uint32_t parsed;
    if (!parse_word_field(route_line, "parent", value, sizeof(value))) {
        return NO_PARENT;
    }
    if (strcmp(value, "none") == 0) {
        return NO_PARENT;
    }
    if (parse_u32_field(route_line, "parent", &parsed)) {
        return parsed;
    }
    return NO_PARENT;
}

static Status read_name_payload(ParserCursor *cursor, const char **name) {
    const char *line;
    if (cursor_next(cursor, &line) != STATUS_OK) {
        return STATUS_ERR;
    }
    if (!starts_with(line, "NAME ")) {
        return STATUS_ERR;
    }
    *name = line + 5;
    return STATUS_OK;
}

static Status parse_header(ParserCursor *cursor, RouteDraft *draft) {
    const char *route_line;
    const char *name_payload;
    HeaderFrame frame;
    uint32_t route_id = 0;
    uint32_t name_len = 0;
    uint32_t declared_nodes = 0;
    char mode[MODE_NAME_MAX + 1];

    if (cursor_next(cursor, &route_line) != STATUS_OK) {
        return STATUS_ERR;
    }
    if (!starts_with(route_line, "ROUTE ")) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(route_line, "id", &route_id)) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(route_line, "name_len", &name_len)) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(route_line, "nodes", &declared_nodes)) {
        return STATUS_ERR;
    }
    if (!parse_word_field(route_line, "mode", mode, sizeof(mode))) {
        return STATUS_ERR;
    }
    if (read_name_payload(cursor, &name_payload) != STATUS_OK) {
        return STATUS_ERR;
    }

    header_frame_init(&frame);
    frame.parent_index = parse_parent_token(route_line);
    frame.declared_nodes = declared_nodes;
    memcpy(frame.mode, mode, strlen(mode) + 1);

    /*
     * Bug: the parser trusts name_len from the file when copying into a
     * stack-resident header frame. Normal route names fit inside label.
     * The trigger declares a larger length and overwrites parent_index and
     * declared_nodes before the frame is copied into the heap route object.
     */
    memcpy(frame.label, name_payload, name_len);
    frame.label[ROUTE_NAME_MAX] = '\0';
    frame.header_checksum = simple_checksum(frame.label);

    if (cursor->trace) {
        printf("trace: route header line='%s'\n", route_line);
        printf("trace: declared name_len=%u raw_payload_len=%zu\n", name_len, strlen(name_payload));
        printf("trace: stack HeaderFrame size=%zu label_addr=%p parent_addr=%p declared_addr=%p\n",
               sizeof(frame),
               (void *)frame.label,
               (void *)&frame.parent_index,
               (void *)&frame.declared_nodes);
        trace_hex("header frame after name copy", &frame, sizeof(frame));
        printf("trace: header fields after copy parent_index=%u declared_nodes=%u mode=%s checksum=%u\n",
               frame.parent_index,
               frame.declared_nodes,
               frame.mode,
               frame.header_checksum);
    }

    route_set_header(draft,
                     route_id,
                     frame.label,
                     frame.mode,
                     frame.parent_index,
                     frame.declared_nodes);
    return STATUS_OK;
}

static Status parse_node_line(const char *line, RouteDraft *draft) {
    uint32_t id = 0;
    uint32_t weight = 0;
    char name[NODE_NAME_MAX + 1];
    if (!starts_with(line, "NODE ")) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(line, "id", &id)) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(line, "weight", &weight)) {
        return STATUS_ERR;
    }
    if (!parse_word_field(line, "name", name, sizeof(name))) {
        return STATUS_ERR;
    }
    return route_add_node(draft, id, name, weight);
}

static Status parse_segment_line(const char *line, RouteDraft *draft) {
    uint32_t from = 0;
    uint32_t to = 0;
    uint32_t cost = 0;
    char label[MODE_NAME_MAX + 1];
    if (!starts_with(line, "SEG ")) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(line, "from", &from)) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(line, "to", &to)) {
        return STATUS_ERR;
    }
    if (!parse_u32_field(line, "cost", &cost)) {
        return STATUS_ERR;
    }
    if (!parse_word_field(line, "label", label, sizeof(label))) {
        return STATUS_ERR;
    }
    return route_add_segment(draft, from, to, cost, label);
}

static Status parse_body(ParserCursor *cursor, RouteDraft *draft) {
    const char *line;
    while (cursor_next(cursor, &line) == STATUS_OK) {
        if (strcmp(line, "END") == 0) {
            return STATUS_OK;
        }
        if (starts_with(line, "NODE ")) {
            if (parse_node_line(line, draft) != STATUS_OK) {
                return STATUS_ERR;
            }
        } else if (starts_with(line, "SEG ")) {
            if (parse_segment_line(line, draft) != STATUS_OK) {
                return STATUS_ERR;
            }
        } else {
            return STATUS_ERR;
        }
    }
    return STATUS_ERR;
}

Status parse_route_file(const char *path, const ParseOptions *options, Route **out_route) {
    LineList lines;
    ParserCursor cursor;
    RouteDraft draft;
    Route *route;
    if (read_lines(path, &lines) != STATUS_OK) {
        fprintf(stderr, "could not read %s\n", path);
        return STATUS_ERR;
    }
    cursor.lines = &lines;
    cursor.pos = 0;
    cursor.trace = options != NULL && options->trace;
    route_draft_init(&draft);
    if (parse_header(&cursor, &draft) != STATUS_OK || parse_body(&cursor, &draft) != STATUS_OK) {
        fprintf(stderr, "parse failed near line %zu\n", cursor.pos);
        route_draft_free(&draft);
        line_list_free(&lines);
        return STATUS_ERR;
    }
    route = route_draft_seal(&draft);
    route_draft_free(&draft);
    line_list_free(&lines);
    if (route == NULL) {
        return STATUS_ERR;
    }
    *out_route = route;
    return STATUS_OK;
}
