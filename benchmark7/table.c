#include "table.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t narrow_count_for_storage(uint32_t declared_count) {
    /* BUG: this silently truncates the 32-bit parsed tile count. */
    uint16_t narrowed = (uint16_t)declared_count;
    return (uint32_t)narrowed;
}

TileTable table_create(const AtlasSpec *spec, const Trace *trace) {
    TileTable table;
    memset(&table, 0, sizeof(table));

    uint32_t alloc_slots = narrow_count_for_storage(spec->total_tiles);
    if (alloc_slots == 0U) {
        alloc_slots = 1U;
    }

    size_t bytes = (size_t)alloc_slots * sizeof(TileSlot);
    table.storage = guarded_alloc(bytes);
    table.slots = (TileSlot *)table.storage.user;
    table.declared_slots = spec->total_tiles;
    table.allocated_slots = alloc_slots;
    table.tile_width = spec->tile_width;
    table.tile_height = spec->tile_height;

    tracef(trace, "table_create declared_slots=%u allocated_slots=%u slot_size=%zu bytes=%zu", table.declared_slots, table.allocated_slots, sizeof(TileSlot), bytes);
    tracef(trace, "table storage=%p requested=%zu mapped=%zu page=%zu", table.storage.user, table.storage.requested_bytes, table.storage.mapped_bytes, table.storage.page_size);
    return table;
}

void table_destroy(TileTable *table) {
    if (table == NULL) {
        return;
    }
    guarded_free(&table->storage);
    memset(table, 0, sizeof(*table));
}

void table_set(TileTable *table, uint32_t index, const TileSlot *slot) {
    if (index >= table->declared_slots) {
        fatal("tile index %u exceeds declared table size %u", index, table->declared_slots);
    }
    table->slots[index] = *slot;
}

const TileSlot *table_get(const TileTable *table, uint32_t index) {
    if (index >= table->declared_slots) {
        fatal("tile lookup %u exceeds declared table size %u", index, table->declared_slots);
    }
    return &table->slots[index];
}

uint32_t table_checksum(const TileTable *table, const Trace *trace) {
    uint32_t acc = 2166136261U;
    uint32_t step = table->declared_slots / 16U;
    if (step == 0U) {
        step = 1U;
    }

    for (uint32_t i = 0U; i < table->declared_slots; i += step) {
        const TileSlot *slot = table_get(table, i);
        acc ^= slot->tile_id;
        acc *= 16777619U;
        acc ^= slot->checksum;
        acc *= 16777619U;
        tracef(trace, "checksum sample index=%u id=%u checksum=%u", i, slot->tile_id, slot->checksum);
    }
    return acc;
}
