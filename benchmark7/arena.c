#include "arena.h"
#include "util.h"

#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static size_t page_size_or_die(void) {
    long p = sysconf(_SC_PAGESIZE);
    if (p <= 0) {
        fatal("unable to determine page size");
    }
    return (size_t)p;
}

static size_t round_up(size_t value, size_t unit) {
    if (value == 0U) {
        return unit;
    }
    size_t rem = value % unit;
    if (rem == 0U) {
        return value;
    }
    return value + (unit - rem);
}

GuardedBlock guarded_alloc(size_t bytes) {
    size_t page = page_size_or_die();
    size_t usable = round_up(bytes, page);
    size_t total = usable + page;

    void *base = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) {
        fatal("mmap failed for guarded allocation");
    }

    char *guard = (char *)base + usable;
    if (mprotect(guard, page, PROT_NONE) != 0) {
        munmap(base, total);
        fatal("mprotect failed for guard page");
    }

    memset(base, 0, usable);

    GuardedBlock block;
    block.base = base;
    block.user = base;
    block.requested_bytes = bytes;
    block.mapped_bytes = total;
    block.page_size = page;
    return block;
}

void guarded_free(GuardedBlock *block) {
    if (block == NULL || block->base == NULL) {
        return;
    }
    if (munmap(block->base, block->mapped_bytes) != 0) {
        fatal("munmap failed for guarded allocation");
    }
    memset(block, 0, sizeof(*block));
}
