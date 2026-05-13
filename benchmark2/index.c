#include "index.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void index_reserve(Index *index, size_t needed)
{
    size_t next_cap;

    if (needed <= index->cap) {
        return;
    }

    next_cap = index->cap == 0U ? 8U : index->cap * 2U;
    while (next_cap < needed) {
        next_cap *= 2U;
    }
    index->entries = xrealloc(index->entries, next_cap * sizeof(index->entries[0]));
    memset(index->entries + index->cap,
           0,
           (next_cap - index->cap) * sizeof(index->entries[0]));
    index->cap = next_cap;

    if (index->trace) {
        printf("trace: index reserve cap=%zu entries=%p\n", index->cap, (void *)index->entries);
    }
}

static bool entry_matches(const IndexEntry *entry, const char *key, size_t key_len)
{
    if (entry->key_len != key_len) {
        return false;
    }

    /*
     * The crash usually appears here. The entry keeps a pointer to bytes inside
     * the text buffer. If the text buffer later moves after realloc, this
     * pointer no longer refers to valid storage.
     */
    return memcmp(entry->key_ptr, key, key_len) == 0;
}

void index_init(Index *index, bool trace)
{
    index->entries = NULL;
    index->len = 0U;
    index->cap = 0U;
    index->trace = trace;
}

void index_destroy(Index *index)
{
    free(index->entries);
    index->entries = NULL;
    index->len = 0U;
    index->cap = 0U;
}

void index_put(Index *index,
               const char *key_ptr,
               size_t key_len,
               size_t value_off,
               size_t value_len)
{
    IndexEntry *slot;
    IndexEntry *existing;

    existing = index_find(index, key_ptr, key_len);
    if (existing != NULL) {
        existing->value_off = value_off;
        existing->value_len = value_len;
        if (index->trace) {
            printf("trace: index update key_ptr=%p len=%zu value_off=%zu\n",
                   (const void *)key_ptr,
                   key_len,
                   value_off);
        }
        return;
    }

    index_reserve(index, index->len + 1U);
    slot = &index->entries[index->len];
    slot->key_ptr = key_ptr;
    slot->key_len = key_len;
    slot->value_off = value_off;
    slot->value_len = value_len;
    slot->hash = hash_bytes(key_ptr, key_len);
    slot->hits = 0U;
    index->len++;

    if (index->trace) {
        printf("trace: index insert key_ptr=%p key_len=%zu value_off=%zu value_len=%zu\n",
               (const void *)slot->key_ptr,
               slot->key_len,
               slot->value_off,
               slot->value_len);
    }
}

IndexEntry *index_find(Index *index, const char *key, size_t key_len)
{
    size_t i;
    uint64_t hash = hash_bytes(key, key_len);

    for (i = 0U; i < index->len; i++) {
        IndexEntry *entry = &index->entries[i];
        if (entry->hash == hash && entry_matches(entry, key, key_len)) {
            entry->hits++;
            return entry;
        }
    }
    return NULL;
}

const IndexEntry *index_find_const(const Index *index, const char *key, size_t key_len)
{
    size_t i;
    uint64_t hash = hash_bytes(key, key_len);

    for (i = 0U; i < index->len; i++) {
        const IndexEntry *entry = &index->entries[i];
        if (entry->hash == hash && entry_matches(entry, key, key_len)) {
            return entry;
        }
    }
    return NULL;
}

void index_dump(const Index *index)
{
    size_t i;

    printf("index dump entries=%zu\n", index->len);
    for (i = 0U; i < index->len; i++) {
        const IndexEntry *entry = &index->entries[i];
        printf("  [%zu] key_ptr=%p key_len=%zu value_off=%zu value_len=%zu hits=%u\n",
               i,
               (const void *)entry->key_ptr,
               entry->key_len,
               entry->value_off,
               entry->value_len,
               entry->hits);
    }
}

void index_stats(const Index *index)
{
    printf("index: entries=%zu cap=%zu table=%p\n",
           index->len,
           index->cap,
           (void *)index->entries);
}
