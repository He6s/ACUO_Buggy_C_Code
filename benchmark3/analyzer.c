#include "analyzer.h"

#include <stdio.h>
#include <string.h>

static unsigned long mix_parent_child(const CatalogNode *parent,
                                      const CatalogNode *child)
{
    unsigned long score = parent->checksum ^ child->checksum;
    score += (unsigned long)parent->id * 131UL;
    score += (unsigned long)child->id * 17UL;
    score ^= (unsigned long)child->payload_len << 3;
    return score;
}

static void append_label(char *path,
                         size_t path_size,
                         const CatalogNode *node,
                         const CatalogNode *parent)
{
    size_t used = strlen(path);
    if (used < path_size) {
        (void)snprintf(path + used,
                       path_size - used,
                       "/%s:%u->%u",
                       node->key,
                       parent->id,
                       node->id);
    }
}

static int evaluate_node(const Catalog *catalog,
                         const CatalogNode *node,
                         char *path,
                         size_t path_size,
                         int trace)
{
    const CatalogNode *parent;
    unsigned long score;

    if (node->parent_id == 0) {
        if (trace) {
            fprintf(stderr, "trace: analyzer root id=%u key=%s\n",
                    node->id, node->key);
        }
        return B3_OK;
    }

    parent = catalog_find(catalog, node->parent_id);
    if (trace) {
        fprintf(stderr,
                "trace: analyzer resolve id=%u parent=%u parent_ptr=%p key=%s\n",
                node->id,
                node->parent_id,
                (const void *)parent,
                node->key);
    }

    score = mix_parent_child(parent, node);
    append_label(path, path_size, node, parent);

    if (trace) {
        fprintf(stderr,
                "trace: analyzer score id=%u score=%lu path=%s\n",
                node->id,
                score,
                path);
    }
    return B3_OK;
}

int analyzer_run(const Catalog *catalog, int trace)
{
    char path[256];
    size_t i;

    memset(path, 0, sizeof(path));
    if (trace) {
        fprintf(stderr, "trace: analyzer visiting %zu catalog nodes\n", catalog->count);
    }

    for (i = 0; i < catalog->count; i++) {
        if (evaluate_node(catalog, &catalog->items[i], path, sizeof(path), trace) != B3_OK) {
            return B3_ERR;
        }
    }

    printf("analysis complete: %zu nodes path=%s\n", catalog->count, path);
    return B3_OK;
}
