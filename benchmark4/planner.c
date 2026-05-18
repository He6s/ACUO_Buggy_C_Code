#include "planner.h"

#include <stdio.h>
#include <string.h>

static uint32_t node_index_for_id(const Route *route, uint32_t id, int *found) {
    size_t i;
    for (i = 0; i < route->node_count; i++) {
        if (route->nodes[i].id == id) {
            *found = 1;
            return (uint32_t)i;
        }
    }
    *found = 0;
    return 0;
}

static uint32_t compute_total_cost(const Route *route) {
    uint32_t total = 0;
    size_t i;
    for (i = 0; i < route->segment_count; i++) {
        total += route->segments[i].cost;
    }
    return total;
}

static uint32_t count_detached_nodes(const Route *route) {
    uint32_t detached = 0;
    size_t i;
    for (i = 0; i < route->node_count; i++) {
        int inbound = 0;
        size_t j;
        for (j = 0; j < route->segment_count; j++) {
            if (route->segments[j].to == route->nodes[i].id) {
                inbound = 1;
            }
        }
        if (!inbound && i != 0) {
            detached++;
        }
    }
    return detached;
}

static void verify_segments(const Route *route) {
    size_t i;
    for (i = 0; i < route->segment_count; i++) {
        int found_from = 0;
        int found_to = 0;
        (void)node_index_for_id(route, route->segments[i].from, &found_from);
        (void)node_index_for_id(route, route->segments[i].to, &found_to);
        if (!found_from || !found_to) {
            printf("warning: segment %zu references unknown node\n", i);
        }
    }
}

static uint32_t parent_score(Route *route, const PlanOptions *options) {
    Node *parent;
    uint32_t score;
    if (route->parent_index == NO_PARENT) {
        return 0;
    }

    if (options != NULL && options->trace) {
        printf("trace: planner resolving parent_index=%u node_count=%zu\n",
               route->parent_index,
               route->node_count);
        printf("trace: nodes base=%p sizeof(Node)=%zu\n",
               (void *)route->nodes,
               sizeof(route->nodes[0]));
    }

    /*
     * Planner assumes the parser already validated parent_index. In the
     * trigger, parent_index was overwritten inside the parser stack frame.
     * The crash appears here when the invalid index is finally dereferenced.
     */
    parent = route_node_at(route, route->parent_index);
    if (options != NULL && options->trace) {
        printf("trace: parent candidate pointer=%p\n", (void *)parent);
    }
    score = parent->weight + (uint32_t)strlen(parent->name);
    return score;
}

Status planner_analyze(Route *route, const PlanOptions *options, PlanReport *report) {
    if (route == NULL || report == NULL) {
        return STATUS_ERR;
    }
    memset(report, 0, sizeof(*report));
    verify_segments(route);
    report->total_cost = compute_total_cost(route);
    report->detached_nodes = count_detached_nodes(route);
    report->parent_weight = parent_score(route, options);
    if (options != NULL && options->trace) {
        printf("trace: report total_cost=%u detached_nodes=%u parent_weight=%u\n",
               report->total_cost,
               report->detached_nodes,
               report->parent_weight);
    }
    return STATUS_OK;
}
