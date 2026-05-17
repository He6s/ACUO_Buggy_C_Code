#ifndef CATALOG_H
#define CATALOG_H

#include <stddef.h>

#include "parser.h"

typedef struct CatalogNode {
    unsigned int id;
    unsigned int parent_id;
    char *key;
    unsigned char *payload;
    size_t payload_len;
    unsigned long checksum;
    size_t source_offset;
} CatalogNode;

typedef struct Catalog {
    CatalogNode *items;
    size_t count;
    size_t capacity;
} Catalog;

void catalog_init(Catalog *catalog);
void catalog_free(Catalog *catalog);
int catalog_build_from_doc(Catalog *catalog, const ParsedDoc *doc, int trace);
const CatalogNode *catalog_find(const Catalog *catalog, unsigned int id);
void catalog_dump(const Catalog *catalog);

#endif
