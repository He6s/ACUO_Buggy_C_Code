#include "catalog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void catalog_push(Catalog *catalog, CatalogNode item)
{
    if (catalog->count == catalog->capacity) {
        size_t next_cap = catalog->capacity == 0 ? 8 : catalog->capacity * 2;
        catalog->items = util_xrealloc(catalog->items,
                                       next_cap * sizeof(catalog->items[0]));
        catalog->capacity = next_cap;
    }
    catalog->items[catalog->count++] = item;
}

static CatalogNode copy_node(const ParsedNode *src)
{
    CatalogNode dst;
    memset(&dst, 0, sizeof(dst));
    dst.id = src->id;
    dst.parent_id = src->parent_id;
    dst.key = util_xstrndup((const unsigned char *)src->key, src->key_len);
    dst.payload = util_xmalloc(src->payload_len + 1);
    if (src->payload_len > 0) {
        memcpy(dst.payload, src->payload, src->payload_len);
    }
    dst.payload[src->payload_len] = '\0';
    dst.payload_len = src->payload_len;
    dst.checksum = util_hash_bytes(dst.payload, dst.payload_len);
    dst.source_offset = src->raw_offset;
    return dst;
}

void catalog_init(Catalog *catalog)
{
    catalog->items = NULL;
    catalog->count = 0;
    catalog->capacity = 0;
}

void catalog_free(Catalog *catalog)
{
    size_t i;
    for (i = 0; i < catalog->count; i++) {
        free(catalog->items[i].key);
        free(catalog->items[i].payload);
    }
    free(catalog->items);
    catalog->items = NULL;
    catalog->count = 0;
    catalog->capacity = 0;
}

int catalog_build_from_doc(Catalog *catalog, const ParsedDoc *doc, int trace)
{
    size_t i;
    for (i = 0; i < doc->count; i++) {
        CatalogNode item = copy_node(&doc->nodes[i]);
        if (trace) {
            fprintf(stderr,
                    "trace: catalog add id=%u parent=%u key=%s source_offset=%zu checksum=%lu\n",
                    item.id,
                    item.parent_id,
                    item.key,
                    item.source_offset,
                    item.checksum);
        }
        catalog_push(catalog, item);
    }
    return B3_OK;
}

const CatalogNode *catalog_find(const Catalog *catalog, unsigned int id)
{
    size_t i;
    for (i = 0; i < catalog->count; i++) {
        if (catalog->items[i].id == id) {
            return &catalog->items[i];
        }
    }
    return NULL;
}

void catalog_dump(const Catalog *catalog)
{
    size_t i;
    printf("catalog nodes: %zu\n", catalog->count);
    for (i = 0; i < catalog->count; i++) {
        const CatalogNode *n = &catalog->items[i];
        printf("  id=%u parent=%u key=%s payload_len=%zu offset=%zu\n",
               n->id,
               n->parent_id,
               n->key,
               n->payload_len,
               n->source_offset);
    }
}
