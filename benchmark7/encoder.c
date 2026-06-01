#include "encoder.h"

#include <stdint.h>

static int is_checkpoint(const TilePlan *plan, uint32_t index) {
    for (uint32_t i = 0U; i < plan->checkpoint_count; ++i) {
        if (plan->checkpoints[i] == index) {
            return 1;
        }
    }
    return 0;
}

static uint32_t update_checksum(uint32_t acc, const TileSlot *slot) {
    acc ^= slot->tile_id + 0x9e3779b9U + (acc << 6) + (acc >> 2);
    acc ^= slot->byte_count;
    acc = (acc << 5) | (acc >> 27);
    acc ^= slot->checksum;
    return acc;
}

EncodeStats encoder_store_plan(TileTable *table, const AtlasSpec *spec, const TilePlan *plan, const Trace *trace) {
    EncodeStats stats;
    stats.stored_tiles = 0U;
    stats.checkpoint_hits = 0U;
    stats.rolling_checksum = 0x13572468U;

    tracef(trace, "encoder starting declared=%u allocated=%u", table->declared_slots, table->allocated_slots);

    for (uint32_t i = 0U; i < plan->tile_count; ++i) {
        TileSlot slot = planner_make_slot(plan, spec, i);
        if (is_checkpoint(plan, i)) {
            ++stats.checkpoint_hits;
            tracef(trace, "encoder checkpoint index=%u label=%s byte_count=%u", i, slot.label, slot.byte_count);
        }
        if (i < 4U || i == table->allocated_slots || i == table->allocated_slots + 1U) {
            tracef(trace, "encoder writing index=%u tile_id=%u label=%s", i, slot.tile_id, slot.label);
        }
        table_set(table, i, &slot);
        ++stats.stored_tiles;
        stats.rolling_checksum = update_checksum(stats.rolling_checksum, &slot);
    }

    tracef(trace, "encoder finished stored=%u checkpoints=%u", stats.stored_tiles, stats.checkpoint_hits);
    return stats;
}
