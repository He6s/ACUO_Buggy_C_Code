#ifndef RECORD_H
#define RECORD_H

#include <stddef.h>

typedef struct Record {
    char *key;
    char *value;
    size_t key_len;
    size_t value_len;
    unsigned ttl;
} Record;

Record *record_create(const char *key, size_t key_len,
                      const char *value, size_t value_len,
                      unsigned ttl);
void record_destroy(Record *rec);
void record_debug_print(const Record *rec);

#endif
