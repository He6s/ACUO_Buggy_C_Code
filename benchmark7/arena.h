#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct GuardedBlock {
    void *base;
    void *user;
    size_t requested_bytes;
    size_t mapped_bytes;
    size_t page_size;
} GuardedBlock;

GuardedBlock guarded_alloc(size_t bytes);
void guarded_free(GuardedBlock *block);

#endif
