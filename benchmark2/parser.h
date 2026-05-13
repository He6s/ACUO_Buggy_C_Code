#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>

#define KEY_MAX 96U
#define VALUE_MAX 512U

typedef enum CommandType {
    CMD_NONE = 0,
    CMD_ADD,
    CMD_GET,
    CMD_FILL,
    CMD_DUMP,
    CMD_STATS,
    CMD_EXIT
} CommandType;

typedef struct Command {
    CommandType type;
    char key[KEY_MAX];
    char value[VALUE_MAX];
    size_t count;
    char fill;
    unsigned line_no;
} Command;

void command_init(Command *cmd, unsigned line_no);
bool parse_command(char *line, unsigned line_no, Command *cmd, char *err, size_t err_cap);
const char *command_name(CommandType type);

#endif
