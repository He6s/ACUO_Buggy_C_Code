#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>

#include "record.h"

typedef enum CommandType {
    CMD_INVALID = 0,
    CMD_ADD,
    CMD_FIND,
    CMD_DEL,
    CMD_DUMP,
    CMD_HELP
} CommandType;

typedef struct Command {
    CommandType type;
    Record *record;
    char *lookup_key;
    size_t lookup_key_len;
    unsigned line_no;
} Command;

bool parser_read_command(FILE *fp, unsigned line_no, Command *out_cmd);
void command_dispose(Command *cmd);
const char *command_type_name(CommandType type);

#endif
