#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse_column_spec(const char *token, char *field, size_t field_size,
                              char *label, size_t label_size, ColumnKind *kind)
{
    char local[MAX_TEXT];
    char *colon1;
    char *colon2;

    copy_text(local, sizeof(local), token);
    colon1 = strchr(local, ':');
    if (colon1 == NULL) {
        return false;
    }
    *colon1 = '\0';
    colon1++;
    colon2 = strchr(colon1, ':');
    if (colon2 == NULL) {
        return false;
    }
    *colon2 = '\0';
    colon2++;
    if (local[0] == '\0' || colon1[0] == '\0' || colon2[0] == '\0') {
        return false;
    }
    copy_text(field, field_size, local);
    copy_text(label, label_size, colon1);
    if (strcmp(colon2, "string") == 0) {
        *kind = COLUMN_STRING;
    } else if (strcmp(colon2, "uint") == 0) {
        *kind = COLUMN_UINT;
    } else if (strcmp(colon2, "flag") == 0) {
        *kind = COLUMN_FLAG;
    } else {
        return false;
    }
    return true;
}

void template_init(Template *tmpl, const char *name)
{
    memset(tmpl, 0, sizeof(*tmpl));
    copy_text(tmpl->name, sizeof(tmpl->name), name);
}

void template_destroy(Template *tmpl)
{
    free(tmpl->columns);
    tmpl->columns = NULL;
    tmpl->len = 0U;
    tmpl->cap = 0U;
}

void template_add_column(Template *tmpl, const char *field, const char *label, ColumnKind expected)
{
    Column *column;

    if (tmpl->len == tmpl->cap) {
        size_t next = tmpl->cap == 0U ? 8U : tmpl->cap * 2U;
        tmpl->columns = xrealloc(tmpl->columns, next * sizeof(tmpl->columns[0]));
        tmpl->cap = next;
    }
    column = &tmpl->columns[tmpl->len];
    tmpl->len++;
    memset(column, 0, sizeof(*column));
    copy_text(column->field_name, sizeof(column->field_name), field);
    copy_text(column->label, sizeof(column->label), label);
    column->expected = expected;
}

void template_copy(Template *dst, const Template *src)
{
    size_t i;

    template_init(dst, src->name);
    for (i = 0U; i < src->len; i++) {
        template_add_column(dst,
                            src->columns[i].field_name,
                            src->columns[i].label,
                            src->columns[i].expected);
    }
}

const Column *template_column(const Template *tmpl, size_t index)
{
    if (index >= tmpl->len) {
        return NULL;
    }
    return &tmpl->columns[index];
}

void command_init(Command *cmd)
{
    memset(cmd, 0, sizeof(*cmd));
    cmd->kind = CMD_NONE;
    template_init(&cmd->tmpl, "");
}

void command_destroy(Command *cmd)
{
    template_destroy(&cmd->tmpl);
    cmd->kind = CMD_NONE;
}

const char *column_kind_name(ColumnKind kind)
{
    switch (kind) {
    case COLUMN_STRING:
        return "string";
    case COLUMN_UINT:
        return "uint";
    case COLUMN_FLAG:
        return "flag";
    default:
        return "unknown";
    }
}

static bool parse_row_command(char *cursor, Command *cmd, char *error, size_t error_size)
{
    char name[MAX_NAME];

    if (!split_token(&cursor, name, sizeof(name))) {
        copy_text(error, error_size, "ROW requires a name");
        return false;
    }
    cmd->kind = CMD_ROW;
    copy_text(cmd->arg1, sizeof(cmd->arg1), name);
    return true;
}

static bool parse_field_command(char *cursor, Command *cmd, char *error, size_t error_size)
{
    char kind[MAX_NAME];
    char name[MAX_NAME];
    char value[MAX_TEXT];
    char *rest;
    unsigned int parsed;

    if (!split_token(&cursor, kind, sizeof(kind)) || !split_token(&cursor, name, sizeof(name))) {
        copy_text(error, error_size, "FIELD requires type and name");
        return false;
    }
    rest = trim_left(cursor);
    trim_right(rest);
    if (rest[0] == '\0') {
        copy_text(error, error_size, "FIELD requires a value");
        return false;
    }
    copy_text(value, sizeof(value), rest);
    copy_text(cmd->arg1, sizeof(cmd->arg1), name);
    if (strcmp(kind, "string") == 0) {
        cmd->kind = CMD_FIELD_STRING;
        copy_text(cmd->arg2, sizeof(cmd->arg2), value);
    } else if (strcmp(kind, "uint") == 0) {
        if (!parse_uint(value, &parsed)) {
            copy_text(error, error_size, "bad unsigned integer field");
            return false;
        }
        cmd->kind = CMD_FIELD_UINT;
        cmd->number = parsed;
    } else if (strcmp(kind, "flag") == 0) {
        cmd->kind = CMD_FIELD_FLAG;
        if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
            cmd->flag = true;
        } else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
            cmd->flag = false;
        } else {
            copy_text(error, error_size, "bad flag field");
            return false;
        }
    } else {
        copy_text(error, error_size, "unknown FIELD type");
        return false;
    }
    return true;
}

static bool parse_template_command(char *cursor, Command *cmd, char *error, size_t error_size)
{
    char name[MAX_NAME];
    char token[MAX_TEXT];

    if (!split_token(&cursor, name, sizeof(name))) {
        copy_text(error, error_size, "TEMPLATE requires a name");
        return false;
    }
    cmd->kind = CMD_TEMPLATE;
    template_destroy(&cmd->tmpl);
    template_init(&cmd->tmpl, name);
    while (split_token(&cursor, token, sizeof(token))) {
        char field[MAX_NAME];
        char label[MAX_NAME];
        ColumnKind kind;
        if (!parse_column_spec(token, field, sizeof(field), label, sizeof(label), &kind)) {
            copy_text(error, error_size, "bad TEMPLATE column spec");
            return false;
        }
        template_add_column(&cmd->tmpl, field, label, kind);
    }
    if (cmd->tmpl.len == 0U) {
        copy_text(error, error_size, "TEMPLATE requires at least one column");
        return false;
    }
    return true;
}

static bool parse_emit_command(char *cursor, Command *cmd, char *error, size_t error_size)
{
    char name[MAX_NAME];

    if (!split_token(&cursor, name, sizeof(name))) {
        copy_text(error, error_size, "EMIT requires template name");
        return false;
    }
    cmd->kind = CMD_EMIT;
    copy_text(cmd->arg1, sizeof(cmd->arg1), name);
    return true;
}

bool parse_line(char *line, Command *cmd, char *error, size_t error_size)
{
    char *cursor;
    char verb[MAX_NAME];

    command_destroy(cmd);
    command_init(cmd);
    error[0] = '\0';
    trim_right(line);
    cursor = trim_left(line);
    if (cursor[0] == '\0' || cursor[0] == '#') {
        cmd->kind = CMD_NONE;
        return true;
    }
    if (!split_token(&cursor, verb, sizeof(verb))) {
        cmd->kind = CMD_NONE;
        return true;
    }
    if (strcmp(verb, "ROW") == 0) {
        return parse_row_command(cursor, cmd, error, error_size);
    }
    if (strcmp(verb, "FIELD") == 0) {
        return parse_field_command(cursor, cmd, error, error_size);
    }
    if (strcmp(verb, "TEMPLATE") == 0) {
        return parse_template_command(cursor, cmd, error, error_size);
    }
    if (strcmp(verb, "EMIT") == 0) {
        return parse_emit_command(cursor, cmd, error, error_size);
    }
    if (strcmp(verb, "DUMP") == 0) {
        cmd->kind = CMD_DUMP;
        return true;
    }
    snprintf(error, error_size, "unknown command '%s'", verb);
    return false;
}
