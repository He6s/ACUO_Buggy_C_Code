#include "store.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct Entry {
    const char *key;
    size_t key_len;
    const char *value;
    size_t value_len;
    unsigned ttl;
    uint64_t hash;
    bool occupied;
    bool tombstone;
} Entry;

struct Store {
    Entry *entries;
    size_t capacity;
    size_t size;
    size_t tombstones;
};

static size_t round_up_capacity(size_t requested) {
    size_t cap = 4;
    while (cap < requested) {
        cap *= 2;
    }
    return cap;
}

static Entry *entry_array_create(size_t capacity) {
    return xcalloc(capacity, sizeof(Entry));
}

static uint64_t hash_key(const char *key, size_t key_len) {
    return fnv1a_bytes(key, key_len);
}

static bool entry_key_equals(const Entry *entry, const char *key, size_t key_len) {
    if (!entry->occupied) {
        return false;
    }
    if (entry->key_len != key_len) {
        return false;
    }
    return memcmp(entry->key, key, key_len) == 0;
}

static size_t slot_for_hash(uint64_t hash, size_t capacity) {
    return (size_t)(hash & (uint64_t)(capacity - 1U));
}

static double load_factor(const Store *store) {
    return (double)(store->size + store->tombstones) / (double)store->capacity;
}

static void entry_reset(Entry *entry) {
    entry->key = NULL;
    entry->key_len = 0;
    entry->value = NULL;
    entry->value_len = 0;
    entry->ttl = 0;
    entry->hash = 0;
    entry->occupied = false;
    entry->tombstone = false;
}

static void entry_assign_borrowed(Entry *entry,
                                  const char *key,
                                  size_t key_len,
                                  const char *value,
                                  size_t value_len,
                                  unsigned ttl,
                                  uint64_t hash) {
    entry->key = key;
    entry->key_len = key_len;
    entry->value = value;
    entry->value_len = value_len;
    entry->ttl = ttl;
    entry->hash = hash;
    entry->occupied = true;
    entry->tombstone = false;
}

static size_t find_slot(const Store *store,
                        const char *key,
                        size_t key_len,
                        uint64_t hash,
                        bool *found_existing) {
    size_t index = slot_for_hash(hash, store->capacity);
    size_t first_tombstone = (size_t)-1;

    for (;;) {
        const Entry *entry = &store->entries[index];

        if (!entry->occupied) {
            if (!entry->tombstone) {
                *found_existing = false;
                if (first_tombstone != (size_t)-1) {
                    return first_tombstone;
                }
                return index;
            }

            if (first_tombstone == (size_t)-1) {
                first_tombstone = index;
            }
        } else if (entry->hash == hash && entry_key_equals(entry, key, key_len)) {
            *found_existing = true;
            return index;
        }

        index = (index + 1U) & (store->capacity - 1U);
    }
}

static void reinsert_entry(Entry *dst_entries, size_t dst_capacity, const Entry *src) {
    size_t index = slot_for_hash(src->hash, dst_capacity);

    while (dst_entries[index].occupied) {
        index = (index + 1U) & (dst_capacity - 1U);
    }

    dst_entries[index] = *src;
}

static void store_grow(Store *store) {
    size_t new_capacity = store->capacity * 2U;
    Entry *new_entries = entry_array_create(new_capacity);
    size_t i;

    for (i = 0; i < store->capacity; ++i) {
        Entry *entry = &store->entries[i];
        if (!entry->occupied) {
            continue;
        }

        entry->hash = hash_key(entry->key, entry->key_len);
        reinsert_entry(new_entries, new_capacity, entry);
    }

    free(store->entries);
    store->entries = new_entries;
    store->capacity = new_capacity;
    store->tombstones = 0;
}

Store *store_create(size_t initial_capacity) {
    Store *store = xcalloc(1, sizeof(*store));

    store->capacity = round_up_capacity(initial_capacity);
    store->entries = entry_array_create(store->capacity);
    store->size = 0;
    store->tombstones = 0;
    return store;
}

void store_destroy(Store *store) {
    size_t i;

    if (store == NULL) {
        return;
    }

    for (i = 0; i < store->capacity; ++i) {
        Entry *entry = &store->entries[i];
        if (!entry->occupied) {
            continue;
        }

        if (entry->key != NULL) {
            page_strfree((char *)entry->key);
        }
        if (entry->value != NULL) {
            page_strfree((char *)entry->value);
        }
        entry_reset(entry);
    }

    free(store->entries);
    free(store);
}

bool store_insert(Store *store, const char *key, size_t key_len,
                  const char *value, size_t value_len, unsigned ttl) {
    bool found_existing = false;
    uint64_t hash;
    size_t index;

    if (store == NULL || key == NULL || value == NULL) {
        return false;
    }

    if (load_factor(store) > 0.60) {
        store_grow(store);
    }

    hash = hash_key(key, key_len);
    index = find_slot(store, key, key_len, hash, &found_existing);

    if (found_existing) {
        Entry *entry = &store->entries[index];
        entry->value = value;
        entry->value_len = value_len;
        entry->ttl = ttl;
        return true;
    }

    if (store->entries[index].tombstone) {
        store->tombstones--;
    }

    entry_assign_borrowed(&store->entries[index], key, key_len, value, value_len, ttl, hash);
    store->size++;
    return true;
}

const char *store_find(const Store *store, const char *key, size_t key_len) {
    uint64_t hash;
    size_t index;

    if (store == NULL || key == NULL) {
        return NULL;
    }

    hash = hash_key(key, key_len);
    index = slot_for_hash(hash, store->capacity);

    for (;;) {
        const Entry *entry = &store->entries[index];

        if (!entry->occupied) {
            if (!entry->tombstone) {
                return NULL;
            }
        } else if (entry->hash == hash && entry_key_equals(entry, key, key_len)) {
            return entry->value;
        }

        index = (index + 1U) & (store->capacity - 1U);
    }
}

bool store_delete(Store *store, const char *key, size_t key_len) {
    bool found_existing = false;
    uint64_t hash;
    size_t index;
    Entry *entry;

    if (store == NULL || key == NULL) {
        return false;
    }

    hash = hash_key(key, key_len);
    index = find_slot(store, key, key_len, hash, &found_existing);
    if (!found_existing) {
        return false;
    }

    entry = &store->entries[index];
    if (entry->key != NULL) {
        page_strfree((char *)entry->key);
    }
    if (entry->value != NULL) {
        page_strfree((char *)entry->value);
    }
    entry_reset(entry);
    entry->tombstone = true;
    store->size--;
    store->tombstones++;
    return true;
}

void store_dump(const Store *store, FILE *out) {
    size_t i;

    fprintf(out, "Store{size=%zu capacity=%zu tombstones=%zu}\n",
            store->size, store->capacity, store->tombstones);

    for (i = 0; i < store->capacity; ++i) {
        const Entry *entry = &store->entries[i];
        fprintf(out, "  [%02zu] occ=%d tomb=%d hash=%" PRIu64,
                i, entry->occupied ? 1 : 0, entry->tombstone ? 1 : 0, entry->hash);
        if (entry->occupied) {
            fprintf(out, " key=%p key_len=%zu value=%p value_len=%zu ttl=%u",
                    (const void *)entry->key, entry->key_len,
                    (const void *)entry->value, entry->value_len,
                    entry->ttl);
        }
        fputc('\n', out);
    }
}

size_t store_size(const Store *store) {
    return store != NULL ? store->size : 0U;
}
