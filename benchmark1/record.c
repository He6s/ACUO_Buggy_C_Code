#include "record.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

Record *record_create(const char *key, size_t key_len,
                      const char *value, size_t value_len,
                      unsigned ttl) {
    Record *rec = xcalloc(1, sizeof(*rec));

    rec->key = page_strdup_len(key, key_len);
    rec->value = page_strdup_len(value, value_len);
    rec->key_len = key_len;
    rec->value_len = value_len;
    rec->ttl = ttl;
    return rec;
}

void record_destroy(Record *rec) {
    if (rec == NULL) {
        return;
    }

    page_str_poison(rec->key);
    page_str_poison(rec->value);
    rec->key = NULL;
    rec->value = NULL;
    rec->key_len = 0;
    rec->value_len = 0;
    rec->ttl = 0;
    free(rec);
}

void record_debug_print(const Record *rec) {
    if (rec == NULL) {
        puts("<null record>");
        return;
    }

    printf("Record{key=%p key_len=%zu value=%p value_len=%zu ttl=%u}\n",
           (void *)rec->key, rec->key_len,
           (void *)rec->value, rec->value_len,
           rec->ttl);
}
