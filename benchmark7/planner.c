#include "planner.h"

#include <stdio.h>
#include <string.h>

static uint32_t mix_index(uint32_t seed, uint32_t index) {
    uint32_t x = seed ^ (index * 2654435761U);
    x ^= x >> 16;
    x *= 2246822519U;
    x ^= x >> 13;
    x *= 3266489917U;
    x ^= x >> 16;
    return x;
}

static uint32_t tile_payload_size(const AtlasSpec *spec, uint32_t index) {
    uint32_t pixels = checked_mul_u32(spec->tile_width, spec->tile_height, "tile pixel count");
    uint32_t base = checked_mul_u32(pixels, spec->layers, "tile byte count");
    uint32_t jitter = (index % 7U) * 3U;
    return base + jitter;
}

TilePlan planner_build(const AtlasSpec *spec, const Trace *trace) {
    TilePlan plan;
    memset(&plan, 0, sizeof(plan));
    plan.tile_count = spec->total_tiles;
    plan.layers = spec->layers;
    plan.seed = spec->seed;
    plan.checkpoint_count = spec->checkpoints;

    uint32_t spacing = plan.tile_count / (plan.checkpoint_count + 1U);
    if (spacing == 0U) {
        spacing = 1U;
    }

    for (uint32_t i = 0U; i < plan.checkpoint_count; ++i) {
        plan.checkpoints[i] = spacing * (i + 1U);
        if (plan.checkpoints[i] >= plan.tile_count) {
            plan.checkpoints[i] = plan.tile_count - 1U;
        }
        tracef(trace, "plan checkpoint[%u]=%u", i, plan.checkpoints[i]);
    }

    tracef(trace, "planner tile_count=%u layers=%u seed=%u", plan.tile_count, plan.layers, plan.seed);
    return plan;
}

TileSlot planner_make_slot(const TilePlan *plan, const AtlasSpec *spec, uint32_t index) {
    TileSlot slot;
    memset(&slot, 0, sizeof(slot));

    uint32_t mixed = mix_index(plan->seed, index);
    uint32_t x = index % spec->grid_width;
    uint32_t y = index / spec->grid_width;
    uint32_t byte_count = tile_payload_size(spec, index);

    slot.tile_id = index + 1U;
    slot.x = x;
    slot.y = y;
    slot.layer = mixed % plan->layers;
    slot.byte_offset = index * 16U + (mixed & 15U);
    slot.byte_count = byte_count;
    slot.checksum = mixed ^ byte_count ^ (x << 8) ^ y;
    snprintf(slot.label, sizeof(slot.label), "tile_%05u_L%u", index, slot.layer);
    return slot;
}

void planner_describe_checkpoint(const TilePlan *plan, uint32_t checkpoint_index, const Trace *trace) {
    if (checkpoint_index >= plan->checkpoint_count) {
        return;
    }
    tracef(trace, "checkpoint %u will validate tile index %u", checkpoint_index, plan->checkpoints[checkpoint_index]);
}
