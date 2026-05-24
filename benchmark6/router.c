#include "router.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void template_book_init(TemplateBook *book)
{
    book->items = NULL;
    book->len = 0U;
    book->cap = 0U;
}

static void template_book_destroy(TemplateBook *book)
{
    size_t i;
    for (i = 0U; i < book->len; i++) {
        template_destroy(&book->items[i]);
    }
    free(book->items);
    book->items = NULL;
    book->len = 0U;
    book->cap = 0U;
}

static Template *template_book_next(TemplateBook *book)
{
    Template *slot;
    if (book->len == book->cap) {
        size_t next = book->cap == 0U ? 4U : book->cap * 2U;
        book->items = xrealloc(book->items, next * sizeof(book->items[0]));
        book->cap = next;
    }
    slot = &book->items[book->len];
    book->len++;
    memset(slot, 0, sizeof(*slot));
    return slot;
}

const Template *template_book_find(const TemplateBook *book, const char *name)
{
    size_t i;
    for (i = 0U; i < book->len; i++) {
        if (strcmp(book->items[i].name, name) == 0) {
            return &book->items[i];
        }
    }
    return NULL;
}

static void template_book_store(TemplateBook *book, const Template *src)
{
    Template *slot = template_book_next(book);
    template_copy(slot, src);
}

void router_init(Router *router, bool trace)
{
    memset(router, 0, sizeof(*router));
    router->trace = trace;
    template_book_init(&router->book);
}

void router_destroy(Router *router)
{
    if (router->have_row) {
        row_destroy(&router->row);
        router->have_row = false;
    }
    template_book_destroy(&router->book);
}

static bool begin_row(Router *router, const char *name)
{
    if (router->have_row) {
        row_destroy(&router->row);
    }
    row_init(&router->row, name);
    router->have_row = true;
    if (router->trace) {
        printf("trace: begin row '%s'\n", name);
    }
    return true;
}

static void append_label(Packet *packet, const Column *column)
{
    packet_add(packet, "s=", column->label);
}

static bool column_matches_field(const Column *column, const Field *field)
{
    if (column->expected == COLUMN_STRING && field->kind == FIELD_STRING) {
        return true;
    }
    if (column->expected == COLUMN_UINT && field->kind == FIELD_UINT) {
        return true;
    }
    if (column->expected == COLUMN_FLAG && field->kind == FIELD_FLAG) {
        return true;
    }
    return false;
}

static void append_field_value(Packet *packet, const Column *column, const Field *field)
{
    if (column->expected == COLUMN_STRING) {
        packet_add(packet, "s", field->text);
    } else if (column->expected == COLUMN_UINT) {
        /*
         * Intentional benchmark bug:
         * The column schema says this slot is an unsigned integer, but this
         * route reuses the string append code path and marks the variadic
         * argument as 's'. packet_add will pull the next vararg as char * and
         * later strlen will dereference the numeric field as an address.
         */
        packet_add(packet, "s", field->number);
    } else if (column->expected == COLUMN_FLAG) {
        packet_add(packet, "b", field->flag ? 1 : 0);
    } else {
        die("bad column kind");
    }
}

static bool emit_template(Router *router, const Template *tmpl)
{
    Packet packet;
    size_t i;

    if (!router->have_row) {
        fprintf(stderr, "no active row\n");
        return false;
    }
    packet_init(&packet, router->trace);
    packet_add(&packet, "s|", "row");
    packet_add(&packet, "s|", router->row.name);
    for (i = 0U; i < tmpl->len; i++) {
        const Column *column = template_column(tmpl, i);
        const Field *field;
        if (column == NULL) {
            packet_destroy(&packet);
            return false;
        }
        field = row_find(&router->row, column->field_name);
        if (field == NULL) {
            fprintf(stderr, "missing field '%s'\n", column->field_name);
            packet_destroy(&packet);
            return false;
        }
        if (!column_matches_field(column, field)) {
            fprintf(stderr, "field '%s' type mismatch: column wants %s, field has %s\n",
                    column->field_name,
                    column_kind_name(column->expected),
                    field_kind_name(field->kind));
            packet_destroy(&packet);
            return false;
        }
        if (router->trace) {
            printf("trace: emit column field='%s' label='%s' expected=%s\n",
                   column->field_name,
                   column->label,
                   column_kind_name(column->expected));
        }
        append_label(&packet, column);
        append_field_value(&packet, column, field);
        packet_add(&packet, "|", 0U);
    }
    packet_add(&packet, "\n", 0U);
    packet_print(&packet);
    router->emitted++;
    packet_destroy(&packet);
    return true;
}

bool router_apply(Router *router, const Command *cmd)
{
    const Template *tmpl;

    switch (cmd->kind) {
    case CMD_NONE:
        return true;
    case CMD_ROW:
        return begin_row(router, cmd->arg1);
    case CMD_FIELD_STRING:
        if (!router->have_row) {
            fprintf(stderr, "FIELD before ROW\n");
            return false;
        }
        row_add_string(&router->row, cmd->arg1, cmd->arg2);
        return true;
    case CMD_FIELD_UINT:
        if (!router->have_row) {
            fprintf(stderr, "FIELD before ROW\n");
            return false;
        }
        row_add_uint(&router->row, cmd->arg1, cmd->number);
        return true;
    case CMD_FIELD_FLAG:
        if (!router->have_row) {
            fprintf(stderr, "FIELD before ROW\n");
            return false;
        }
        row_add_flag(&router->row, cmd->arg1, cmd->flag);
        return true;
    case CMD_TEMPLATE:
        template_book_store(&router->book, &cmd->tmpl);
        if (router->trace) {
            printf("trace: stored template '%s' with %zu columns\n", cmd->tmpl.name, cmd->tmpl.len);
        }
        return true;
    case CMD_EMIT:
        tmpl = template_book_find(&router->book, cmd->arg1);
        if (tmpl == NULL) {
            fprintf(stderr, "unknown template '%s'\n", cmd->arg1);
            return false;
        }
        return emit_template(router, tmpl);
    case CMD_DUMP:
        router_dump(router);
        return true;
    default:
        return false;
    }
}

void router_dump(const Router *router)
{
    size_t i;

    printf("router emitted=%u templates=%zu\n", router->emitted, router->book.len);
    if (router->have_row) {
        row_dump(&router->row);
    }
    for (i = 0U; i < router->book.len; i++) {
        const Template *tmpl = &router->book.items[i];
        size_t j;
        printf("template '%s' columns=%zu\n", tmpl->name, tmpl->len);
        for (j = 0U; j < tmpl->len; j++) {
            printf("  %s:%s:%s\n",
                   tmpl->columns[j].field_name,
                   tmpl->columns[j].label,
                   column_kind_name(tmpl->columns[j].expected));
        }
    }
}
