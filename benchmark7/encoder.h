#ifndef ENCODER_H
#define ENCODER_H

#include "parser.h"
#include "planner.h"
#include "table.h"
#include "util.h"

#include <stdint.h>

typedef struct EncodeStats {
    uint32_t stored_tiles;
    uint32_t checkpoint_hits;
    uint32_t rolling_checksum;
} EncodeStats;

EncodeStats encoder_store_plan(TileTable *table, const AtlasSpec *spec, const TilePlan *plan, const Trace *trace);

#endif
