#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum CommandType {
    CMD_TASK,
    CMD_READY,
    CMD_NOTE,
    CMD_RUN,
    CMD_DUMP
} CommandType;

typedef struct Command {
    CommandType type;
    uint32_t id;
    unsigned priority;
    size_t note_len;
    char owner[32];
    char title[64];
    char note_text[160];
    unsigned line_no;
} Command;

typedef struct CommandList {
    Command *items;
    size_t count;
    size_t capacity;
} CommandList;

void command_list_init(CommandList *list);
void command_list_destroy(CommandList *list);
void parse_file(const char *path, CommandList *out, bool trace);
const char *command_type_name(CommandType type);

#endif
