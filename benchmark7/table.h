#ifndef TABLE_H
#define TABLE_H

#include "arena.h"
#include "parser.h"
#include "util.h"

#include <stdint.h>

typedef struct TileSlot {
    uint32_t tile_id;
    uint32_t x;
    uint32_t y;
    uint32_t layer;
    uint32_t byte_offset;
    uint32_t byte_count;
    uint32_t checksum;
    char label[24];
} TileSlot;

typedef struct TileTable {
    TileSlot *slots;
    uint32_t declared_slots;
    uint32_t allocated_slots;
    uint32_t tile_width;
    uint32_t tile_height;
    GuardedBlock storage;
} TileTable;

TileTable table_create(const AtlasSpec *spec, const Trace *trace);
void table_destroy(TileTable *table);
void table_set(TileTable *table, uint32_t index, const TileSlot *slot);
const TileSlot *table_get(const TileTable *table, uint32_t index);
uint32_t table_checksum(const TileTable *table, const Trace *trace);

#endif
