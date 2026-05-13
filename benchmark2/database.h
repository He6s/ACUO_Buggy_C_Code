#ifndef DATABASE_H
#define DATABASE_H

#include "parser.h"
#include "textbuf.h"
#include "index.h"

#include <stdbool.h>

typedef struct Database {
    TextBuf text;
    Index index;
    bool trace;
    unsigned operations;
    unsigned adds;
    unsigned gets;
    unsigned fills;
} Database;

void database_init(Database *db, bool trace);
void database_destroy(Database *db);
bool database_execute(Database *db, const Command *cmd);
void database_add(Database *db, const char *key, const char *value);
void database_get(Database *db, const char *key);
void database_fill(Database *db, size_t count, char fill);
void database_dump(const Database *db);
void database_stats(const Database *db);

#endif
