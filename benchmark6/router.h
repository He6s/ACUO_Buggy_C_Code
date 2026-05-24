#ifndef ROUTER_H
#define ROUTER_H

#include "format.h"
#include "parser.h"
#include "table.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct TemplateBook {
    Template *items;
    size_t len;
    size_t cap;
} TemplateBook;

typedef struct Router {
    Row row;
    TemplateBook book;
    bool have_row;
    bool trace;
    unsigned int emitted;
} Router;

void router_init(Router *router, bool trace);
void router_destroy(Router *router);
bool router_apply(Router *router, const Command *cmd);
void router_dump(const Router *router);
const Template *template_book_find(const TemplateBook *book, const char *name);

#endif
