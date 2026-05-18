#include "parser.h"
#include "planner.h"
#include "route.h"

#include <stdio.h>
#include <string.h>

static void usage(const char *argv0) {
    fprintf(stderr, "usage: %s trigger.txt [--trace]\n", argv0);
}

static int has_trace(int argc, char **argv) {
    int i;
    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--trace") == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    ParseOptions parse_options;
    PlanOptions plan_options;
    PlanReport report;
    Route *route = NULL;

    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    parse_options.trace = has_trace(argc, argv);
    plan_options.trace = parse_options.trace;

    if (parse_route_file(argv[1], &parse_options, &route) != STATUS_OK) {
        return 1;
    }

    if (parse_options.trace) {
        route_print_summary(route);
    }

    if (planner_analyze(route, &plan_options, &report) != STATUS_OK) {
        route_free(route);
        return 1;
    }

    printf("route=%s total_cost=%u detached=%u parent_score=%u\n",
           route->label,
           report.total_cost,
           report.detached_nodes,
           report.parent_weight);

    route_free(route);
    return 0;
}
