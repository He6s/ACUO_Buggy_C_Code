#include "database.h"

#include "util.h"

#include <stdio.h>
#include <string.h>

void database_init(Database *db, bool trace)
{
    db->trace = trace;
    db->operations = 0U;
    db->adds = 0U;
    db->gets = 0U;
    db->fills = 0U;
    textbuf_init(&db->text, trace);
    index_init(&db->index, trace);
}

void database_destroy(Database *db)
{
    index_destroy(&db->index);
    textbuf_destroy(&db->text);
    db->operations = 0U;
    db->adds = 0U;
    db->gets = 0U;
    db->fills = 0U;
}

void database_add(Database *db, const char *key, const char *value)
{
    TextSlice key_slice;
    TextSlice value_slice;

    key_slice = textbuf_append_cstr(&db->text, key);
    value_slice = textbuf_append_value(&db->text, value);

    /*
     * Intentional benchmark bug: the index stores key_slice.ptr directly.
     * key_slice.ptr points inside the text buffer. When the text buffer later
     * grows through realloc, old interior pointers held by the index are not
     * repaired. The value is stored by offset, so it survives relocation.
     */
    index_put(&db->index,
              key_slice.ptr,
              key_slice.len,
              value_slice.off,
              value_slice.len);

    db->adds++;
    if (db->trace) {
        printf("trace: add key='%s' key_ptr=%p key_off=%zu value_off=%zu\n",
               key,
               (void *)key_slice.ptr,
               key_slice.off,
               value_slice.off);
    }
}

void database_get(Database *db, const char *key)
{
    IndexEntry *entry;
    const char *value;
    size_t key_len = strlen(key);

    if (db->trace) {
        printf("trace: get key='%s' current_buffer=%p\n", key, (void *)db->text.data);
    }

    entry = index_find(&db->index, key, key_len);
    if (entry == NULL) {
        printf("MISS %s\n", key);
        db->gets++;
        return;
    }

    value = textbuf_at(&db->text, entry->value_off, entry->value_len);
    printf("HIT %s ", key);
    fwrite(value, 1U, entry->value_len, stdout);
    putchar('\n');
    db->gets++;
}

void database_fill(Database *db, size_t count, char fill)
{
    textbuf_append_fill(&db->text, count, fill);
    db->fills++;
}

void database_dump(const Database *db)
{
    size_t i;

    printf("database dump\n");
    index_dump(&db->index);
    for (i = 0U; i < db->index.len; i++) {
        const IndexEntry *entry = &db->index.entries[i];
        bool current = textbuf_owns_ptr(&db->text, entry->key_ptr);
        printf("  key[%zu] ptr=%p current_buffer=%s value_off=%zu\n",
               i,
               (const void *)entry->key_ptr,
               current ? "yes" : "no",
               entry->value_off);
    }
}

void database_stats(const Database *db)
{
    printf("ops=%u adds=%u gets=%u fills=%u\n",
           db->operations,
           db->adds,
           db->gets,
           db->fills);
    textbuf_stats(&db->text);
    index_stats(&db->index);
}

bool database_execute(Database *db, const Command *cmd)
{
    if (cmd->type == CMD_NONE) {
        return true;
    }

    db->operations++;
    if (db->trace) {
        printf("trace: line=%u cmd=%s\n", cmd->line_no, command_name(cmd->type));
    }

    switch (cmd->type) {
    case CMD_ADD:
        database_add(db, cmd->key, cmd->value);
        return true;
    case CMD_GET:
        database_get(db, cmd->key);
        return true;
    case CMD_FILL:
        database_fill(db, cmd->count, cmd->fill);
        return true;
    case CMD_DUMP:
        database_dump(db);
        return true;
    case CMD_STATS:
        database_stats(db);
        return true;
    case CMD_EXIT:
        return false;
    case CMD_NONE:
        return true;
    default:
        die("unknown command type %d", (int)cmd->type);
    }

    return false;
}
