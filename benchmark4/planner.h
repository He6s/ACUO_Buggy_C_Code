#ifndef PLANNER_H
#define PLANNER_H

#include "route.h"

#include <stdint.h>

typedef struct PlanOptions {
    int trace;
} PlanOptions;

typedef struct PlanReport {
    uint32_t total_cost;
    uint32_t parent_weight;
    uint32_t detached_nodes;
} PlanReport;

Status planner_analyze(Route *route, const PlanOptions *options, PlanReport *report);

#endif
