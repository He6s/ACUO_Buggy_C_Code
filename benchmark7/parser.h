#ifndef PARSER_H
#define PARSER_H

#include "util.h"

#include <stdint.h>

typedef struct AtlasSpec {
    char *name;
    uint32_t tile_width;
    uint32_t tile_height;
    uint32_t grid_width;
    uint32_t grid_height;
    uint32_t layers;
    uint32_t seed;
    uint32_t checkpoints;
    uint32_t total_tiles;
} AtlasSpec;

AtlasSpec parse_atlas_file(const char *path, const Trace *trace);
void free_atlas_spec(AtlasSpec *spec);
void dump_atlas_spec(const AtlasSpec *spec, const Trace *trace);

#endif
