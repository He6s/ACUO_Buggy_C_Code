#include "report.h"

#include <stdio.h>

static void print_human_size(uint32_t bytes) {
    if (bytes >= 1024U * 1024U) {
        printf("%u MiB", bytes / (1024U * 1024U));
    } else if (bytes >= 1024U) {
        printf("%u KiB", bytes / 1024U);
    } else {
        printf("%u B", bytes);
    }
}

void report_summary(const AtlasSpec *spec, const TilePlan *plan, const TileTable *table, const EncodeStats *stats, const Trace *trace) {
    uint32_t table_sig = table_checksum(table, trace);
    uint32_t payload = checked_mul_u32(spec->tile_width, spec->tile_height, "report tile pixels");
    payload = checked_mul_u32(payload, spec->layers, "report payload bytes");

    puts("tilepack summary");
    printf("  atlas: %s\n", spec->name);
    printf("  grid: %u x %u = %u tiles\n", spec->grid_width, spec->grid_height, spec->total_tiles);
    printf("  tile payload: ");
    print_human_size(payload);
    putchar('\n');
    printf("  stored tiles: %u\n", stats->stored_tiles);
    printf("  checkpoints: %u of %u\n", stats->checkpoint_hits, plan->checkpoint_count);
    printf("  allocated slots: %u\n", table->allocated_slots);
    printf("  rolling checksum: %08x\n", stats->rolling_checksum);
    printf("  table signature: %08x\n", table_sig);
}
