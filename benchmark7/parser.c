#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SeenFields {
    int name;
    int tile_width;
    int tile_height;
    int grid_width;
    int grid_height;
    int layers;
    int seed;
    int checkpoints;
    int end;
} SeenFields;

static void assign_string(char **slot, const char *value, const char *field) {
    if (*slot != NULL) {
        fatal("duplicate field %s", field);
    }
    *slot = xstrdup(value);
}

static void set_u32(uint32_t *slot, int *seen, const char *value, const char *field) {
    if (*seen) {
        fatal("duplicate field %s", field);
    }
    *slot = parse_u32_strict(value, field);
    *seen = 1;
}

static void require_fields(const AtlasSpec *spec, const SeenFields *seen) {
    if (!seen->name || spec->name == NULL) {
        fatal("missing NAME field");
    }
    if (!seen->tile_width) {
        fatal("missing TILE_WIDTH field");
    }
    if (!seen->tile_height) {
        fatal("missing TILE_HEIGHT field");
    }
    if (!seen->grid_width) {
        fatal("missing GRID_WIDTH field");
    }
    if (!seen->grid_height) {
        fatal("missing GRID_HEIGHT field");
    }
    if (!seen->layers) {
        fatal("missing LAYERS field");
    }
    if (!seen->seed) {
        fatal("missing SEED field");
    }
    if (!seen->checkpoints) {
        fatal("missing CHECKPOINTS field");
    }
    if (!seen->end) {
        fatal("missing END line");
    }
}

static void validate_spec(const AtlasSpec *spec) {
    if (spec->tile_width == 0U || spec->tile_height == 0U) {
        fatal("tile size must be nonzero");
    }
    if (spec->grid_width == 0U || spec->grid_height == 0U) {
        fatal("grid size must be nonzero");
    }
    if (spec->layers == 0U || spec->layers > 8U) {
        fatal("layers must be between 1 and 8");
    }
    if (spec->checkpoints == 0U || spec->checkpoints > 128U) {
        fatal("checkpoints must be between 1 and 128");
    }
}

static void parse_line(AtlasSpec *spec, SeenFields *seen, char *line, unsigned line_no, const Trace *trace) {
    char *key = NULL;
    char *value = NULL;
    if (!split_key_value(line, &key, &value)) {
        return;
    }

    tracef(trace, "line %u key=%s value=%s", line_no, key, value);

    if (strcmp(key, "NAME") == 0) {
        if (seen->name) {
            fatal("duplicate field NAME");
        }
        assign_string(&spec->name, value, "NAME");
        seen->name = 1;
    } else if (strcmp(key, "TILE_WIDTH") == 0) {
        set_u32(&spec->tile_width, &seen->tile_width, value, "TILE_WIDTH");
    } else if (strcmp(key, "TILE_HEIGHT") == 0) {
        set_u32(&spec->tile_height, &seen->tile_height, value, "TILE_HEIGHT");
    } else if (strcmp(key, "GRID_WIDTH") == 0) {
        set_u32(&spec->grid_width, &seen->grid_width, value, "GRID_WIDTH");
    } else if (strcmp(key, "GRID_HEIGHT") == 0) {
        set_u32(&spec->grid_height, &seen->grid_height, value, "GRID_HEIGHT");
    } else if (strcmp(key, "LAYERS") == 0) {
        set_u32(&spec->layers, &seen->layers, value, "LAYERS");
    } else if (strcmp(key, "SEED") == 0) {
        set_u32(&spec->seed, &seen->seed, value, "SEED");
    } else if (strcmp(key, "CHECKPOINTS") == 0) {
        set_u32(&spec->checkpoints, &seen->checkpoints, value, "CHECKPOINTS");
    } else if (strcmp(key, "END") == 0) {
        if (seen->end) {
            fatal("duplicate END line");
        }
        if (strcmp(value, "ATLAS") != 0) {
            fatal("END must be followed by ATLAS");
        }
        seen->end = 1;
    } else {
        fatal("unknown field on line %u: %s", line_no, key);
    }
}

AtlasSpec parse_atlas_file(const char *path, const Trace *trace) {
    size_t len = 0U;
    char *text = read_text_file(path, &len);
    tracef(trace, "read %zu bytes from %s", len, path);

    AtlasSpec spec;
    memset(&spec, 0, sizeof(spec));
    SeenFields seen;
    memset(&seen, 0, sizeof(seen));

    unsigned line_no = 1U;
    char *cursor = text;
    while (*cursor != '\0') {
        char *line_start = cursor;
        char *newline = strchr(cursor, '\n');
        if (newline != NULL) {
            *newline = '\0';
            cursor = newline + 1;
        } else {
            cursor += strlen(cursor);
        }
        parse_line(&spec, &seen, line_start, line_no, trace);
        ++line_no;
    }

    require_fields(&spec, &seen);
    validate_spec(&spec);
    spec.total_tiles = checked_mul_u32(spec.grid_width, spec.grid_height, "grid tile count");
    tracef(trace, "parser computed total_tiles=%u", spec.total_tiles);

    free(text);
    return spec;
}

void free_atlas_spec(AtlasSpec *spec) {
    if (spec == NULL) {
        return;
    }
    free(spec->name);
    memset(spec, 0, sizeof(*spec));
}

void dump_atlas_spec(const AtlasSpec *spec, const Trace *trace) {
    tracef(trace, "atlas name=%s", spec->name);
    tracef(trace, "tile=%ux%u grid=%ux%u layers=%u", spec->tile_width, spec->tile_height, spec->grid_width, spec->grid_height, spec->layers);
    tracef(trace, "seed=%u checkpoints=%u total_tiles=%u", spec->seed, spec->checkpoints, spec->total_tiles);
}
