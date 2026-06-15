#include "parser.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void command_list_push(CommandList *list, const Command *cmd) {
    if (list->count == list->capacity) {
        size_t new_cap = list->capacity == 0 ? 16 : list->capacity * 2;
        Command *new_items = xcalloc(new_cap, sizeof(*new_items));
        if (list->items != NULL) {
            memcpy(new_items, list->items, list->count * sizeof(*new_items));
            free(list->items);
        }
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = *cmd;
}

static void copy_token(char *dst, size_t cap, const char *src, unsigned line_no) {
    size_t n = strlen(src);
    if (n >= cap) {
        die("line %u: token too long", line_no);
    }
    memcpy(dst, src, n + 1);
}

static char *next_token(char **cursor) {
    char *start;
    char *p;

    *cursor = trim_left(*cursor);
    if (**cursor == '\0') {
        return NULL;
    }
    start = *cursor;
    p = start;
    while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
        p++;
    }
    if (*p != '\0') {
        *p = '\0';
        p++;
    }
    *cursor = p;
    return start;
}

static void parse_task(char *cursor, Command *cmd) {
    char *id_text = next_token(&cursor);
    char *owner = next_token(&cursor);
    char *priority_text = next_token(&cursor);
    char *title = trim_left(cursor);
    uint32_t id;
    uint32_t pri;

    trim_right(title);
    if (id_text == NULL || owner == NULL || priority_text == NULL || *title == '\0') {
        die("line %u: task requires id owner priority title", cmd->line_no);
    }
    if (!parse_u32(id_text, &id) || !parse_u32(priority_text, &pri)) {
        die("line %u: invalid task numeric field", cmd->line_no);
    }
    cmd->type = CMD_TASK;
    cmd->id = id;
    cmd->priority = pri;
    copy_token(cmd->owner, sizeof(cmd->owner), owner, cmd->line_no);
    copy_token(cmd->title, sizeof(cmd->title), title, cmd->line_no);
}

static void parse_ready(char *cursor, Command *cmd) {
    char *id_text = next_token(&cursor);
    uint32_t id;

    if (id_text == NULL || !parse_u32(id_text, &id)) {
        die("line %u: ready requires numeric id", cmd->line_no);
    }
    cmd->type = CMD_READY;
    cmd->id = id;
}

static void parse_note(char *cursor, Command *cmd) {
    char *id_text = next_token(&cursor);
    char *len_text = next_token(&cursor);
    char *payload = trim_left(cursor);
    uint32_t id;
    size_t len;
    size_t available;

    trim_right(payload);
    if (id_text == NULL || len_text == NULL || *payload == '\0') {
        die("line %u: note requires id length text", cmd->line_no);
    }
    if (!parse_u32(id_text, &id) || !parse_size(len_text, &len)) {
        die("line %u: invalid note numeric field", cmd->line_no);
    }
    available = strlen(payload);
    if (len > available) {
        die("line %u: note length %zu exceeds available payload %zu", cmd->line_no, len, available);
    }
    if (available >= sizeof(cmd->note_text)) {
        die("line %u: note payload too long for command buffer", cmd->line_no);
    }
    cmd->type = CMD_NOTE;
    cmd->id = id;
    cmd->note_len = len;
    memcpy(cmd->note_text, payload, available + 1);
}

static void parse_run(Command *cmd) {
    cmd->type = CMD_RUN;
}

static void parse_dump(Command *cmd) {
    cmd->type = CMD_DUMP;
}

void command_list_init(CommandList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void command_list_destroy(CommandList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

const char *command_type_name(CommandType type) {
    switch (type) {
        case CMD_TASK:
            return "task";
        case CMD_READY:
            return "ready";
        case CMD_NOTE:
            return "note";
        case CMD_RUN:
            return "run";
        case CMD_DUMP:
            return "dump";
        default:
            return "unknown";
    }
}

void parse_file(const char *path, CommandList *out, bool trace) {
    size_t file_len;
    char *text = read_entire_file(path, &file_len);
    char *save = text;
    char *line = text;
    unsigned line_no = 1;

    (void)file_len;
    while (line != NULL) {
        char *next = strchr(line, '\n');
        char *cursor;
        char *verb;
        Command cmd;

        if (next != NULL) {
            *next = '\0';
            next++;
        }
        cursor = trim_left(line);
        trim_right(cursor);
        if (*cursor != '\0' && *cursor != '#') {
            memset(&cmd, 0, sizeof(cmd));
            cmd.line_no = line_no;
            verb = next_token(&cursor);
            if (verb == NULL) {
                die("line %u: missing command", line_no);
            }
            if (strcmp(verb, "task") == 0) {
                parse_task(cursor, &cmd);
            } else if (strcmp(verb, "ready") == 0) {
                parse_ready(cursor, &cmd);
            } else if (strcmp(verb, "note") == 0) {
                parse_note(cursor, &cmd);
            } else if (strcmp(verb, "run") == 0) {
                parse_run(&cmd);
            } else if (strcmp(verb, "dump") == 0) {
                parse_dump(&cmd);
            } else {
                die("line %u: unknown command %s", line_no, verb);
            }
            if (trace) {
                fprintf(stderr, "[parse] line=%u type=%s id=%u len=%zu\n",
                        cmd.line_no, command_type_name(cmd.type), cmd.id, cmd.note_len);
            }
            command_list_push(out, &cmd);
        }
        line = next;
        line_no++;
    }
    free(save);
}
