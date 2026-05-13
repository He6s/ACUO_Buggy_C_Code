#ifndef INDEX_H
#define INDEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct IndexEntry {
    const char *key_ptr;
    size_t key_len;
    size_t value_off;
    size_t value_len;
    uint64_t hash;
    unsigned hits;
} IndexEntry;

typedef struct Index {
    IndexEntry *entries;
    size_t len;
    size_t cap;
    bool trace;
} Index;

void index_init(Index *index, bool trace);
void index_destroy(Index *index);
void index_put(Index *index,
               const char *key_ptr,
               size_t key_len,
               size_t value_off,
               size_t value_len);
IndexEntry *index_find(Index *index, const char *key, size_t key_len);
const IndexEntry *index_find_const(const Index *index, const char *key, size_t key_len);
void index_dump(const Index *index);
void index_stats(const Index *index);

#endif
