#ifndef PLANNER_H
#define PLANNER_H

#include "parser.h"
#include "table.h"
#include "util.h"

#include <stdint.h>

typedef struct TilePlan {
    uint32_t tile_count;
    uint32_t layers;
    uint32_t seed;
    uint32_t checkpoint_count;
    uint32_t checkpoints[128];
} TilePlan;

TilePlan planner_build(const AtlasSpec *spec, const Trace *trace);
TileSlot planner_make_slot(const TilePlan *plan, const AtlasSpec *spec, uint32_t index);
void planner_describe_checkpoint(const TilePlan *plan, uint32_t checkpoint_index, const Trace *trace);

#endif
