#ifndef PARSER_H
#define PARSER_H

#include "table.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum ColumnKind {
    COLUMN_STRING,
    COLUMN_UINT,
    COLUMN_FLAG
} ColumnKind;

typedef struct Column {
    char field_name[MAX_NAME];
    char label[MAX_NAME];
    ColumnKind expected;
} Column;

typedef struct Template {
    char name[MAX_NAME];
    Column *columns;
    size_t len;
    size_t cap;
} Template;

typedef enum CommandKind {
    CMD_NONE,
    CMD_ROW,
    CMD_FIELD_STRING,
    CMD_FIELD_UINT,
    CMD_FIELD_FLAG,
    CMD_TEMPLATE,
    CMD_EMIT,
    CMD_DUMP
} CommandKind;

typedef struct Command {
    CommandKind kind;
    char arg1[MAX_TEXT];
    char arg2[MAX_TEXT];
    char arg3[MAX_TEXT];
    unsigned int number;
    bool flag;
    Template tmpl;
} Command;

void template_init(Template *tmpl, const char *name);
void template_destroy(Template *tmpl);
void template_add_column(Template *tmpl, const char *field, const char *label, ColumnKind expected);
void template_copy(Template *dst, const Template *src);
const Column *template_column(const Template *tmpl, size_t index);
void command_init(Command *cmd);
void command_destroy(Command *cmd);
bool parse_line(char *line, Command *cmd, char *error, size_t error_size);
const char *column_kind_name(ColumnKind kind);

#endif
