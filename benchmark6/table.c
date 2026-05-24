#include "table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Field *row_next_field(Row *row, const char *name, FieldKind kind)
{
    Field *field;

    if (row->len == row->cap) {
        size_t next_cap = row->cap == 0U ? 8U : row->cap * 2U;
        row->fields = xrealloc(row->fields, next_cap * sizeof(row->fields[0]));
        row->cap = next_cap;
    }
    field = &row->fields[row->len];
    row->len++;
    memset(field, 0, sizeof(*field));
    copy_text(field->name, sizeof(field->name), name);
    field->kind = kind;
    return field;
}

void row_init(Row *row, const char *name)
{
    memset(row, 0, sizeof(*row));
    copy_text(row->name, sizeof(row->name), name);
}

void row_destroy(Row *row)
{
    free(row->fields);
    row->fields = NULL;
    row->len = 0U;
    row->cap = 0U;
}

void row_add_string(Row *row, const char *name, const char *value)
{
    Field *field = row_next_field(row, name, FIELD_STRING);
    copy_text(field->text, sizeof(field->text), value);
}

void row_add_uint(Row *row, const char *name, unsigned int value)
{
    Field *field = row_next_field(row, name, FIELD_UINT);
    field->number = value;
}

void row_add_flag(Row *row, const char *name, bool value)
{
    Field *field = row_next_field(row, name, FIELD_FLAG);
    field->flag = value;
}

const Field *row_find(const Row *row, const char *name)
{
    size_t i;

    for (i = 0U; i < row->len; i++) {
        if (strcmp(row->fields[i].name, name) == 0) {
            return &row->fields[i];
        }
    }
    return NULL;
}

const char *field_kind_name(FieldKind kind)
{
    switch (kind) {
    case FIELD_STRING:
        return "string";
    case FIELD_UINT:
        return "uint";
    case FIELD_FLAG:
        return "flag";
    default:
        return "unknown";
    }
}

void row_dump(const Row *row)
{
    size_t i;

    printf("row '%s' with %zu fields\n", row->name, row->len);
    for (i = 0U; i < row->len; i++) {
        const Field *field = &row->fields[i];
        printf("  %s kind=%s ", field->name, field_kind_name(field->kind));
        if (field->kind == FIELD_STRING) {
            printf("value='%s'\n", field->text);
        } else if (field->kind == FIELD_UINT) {
            printf("value=%u\n", field->number);
        } else {
            printf("value=%s\n", field->flag ? "true" : "false");
        }
    }
}
