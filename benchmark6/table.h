#ifndef TABLE_H
#define TABLE_H

#include "util.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum FieldKind {
    FIELD_STRING,
    FIELD_UINT,
    FIELD_FLAG
} FieldKind;

typedef struct Field {
    char name[MAX_NAME];
    FieldKind kind;
    char text[MAX_TEXT];
    unsigned int number;
    bool flag;
} Field;

typedef struct Row {
    char name[MAX_NAME];
    Field *fields;
    size_t len;
    size_t cap;
} Row;

void row_init(Row *row, const char *name);
void row_destroy(Row *row);
void row_add_string(Row *row, const char *name, const char *value);
void row_add_uint(Row *row, const char *name, unsigned int value);
void row_add_flag(Row *row, const char *name, bool value);
const Field *row_find(const Row *row, const char *name);
void row_dump(const Row *row);
const char *field_kind_name(FieldKind kind);

#endif
