#include "encoder.h"
#include "parser.h"
#include "planner.h"
#include "report.h"
#include "table.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static void usage(const char *argv0) {
    fprintf(stderr, "usage: %s trigger.txt [--trace]\n", argv0);
}

static int has_trace_flag(int argc, char **argv) {
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--trace") == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return 2;
    }

    Trace trace;
    trace.enabled = has_trace_flag(argc, argv);

    AtlasSpec spec = parse_atlas_file(argv[1], &trace);
    dump_atlas_spec(&spec, &trace);

    TilePlan plan = planner_build(&spec, &trace);
    for (uint32_t i = 0U; i < plan.checkpoint_count && i < 3U; ++i) {
        planner_describe_checkpoint(&plan, i, &trace);
    }

    TileTable table = table_create(&spec, &trace);
    EncodeStats stats = encoder_store_plan(&table, &spec, &plan, &trace);
    report_summary(&spec, &plan, &table, &stats, &trace);

    table_destroy(&table);
    free_atlas_spec(&spec);
    return 0;
}
