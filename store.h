#ifndef STORE_H
#define STORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct Store Store;

Store *store_create(size_t initial_capacity);
void store_destroy(Store *store);
bool store_insert(Store *store, const char *key, size_t key_len,
                  const char *value, size_t value_len, unsigned ttl);
const char *store_find(const Store *store, const char *key, size_t key_len);
bool store_delete(Store *store, const char *key, size_t key_len);
void store_dump(const Store *store, FILE *out);
size_t store_size(const Store *store);

#endif
