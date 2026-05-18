#ifndef ROUTE_H
#define ROUTE_H

#include "util.h"

#include <stddef.h>
#include <stdint.h>

typedef struct Node {
    uint32_t id;
    char name[NODE_NAME_MAX + 1];
    uint32_t weight;
} Node;

typedef struct Segment {
    uint32_t from;
    uint32_t to;
    uint32_t cost;
    char label[MODE_NAME_MAX + 1];
} Segment;

typedef struct Route {
    uint32_t id;
    char label[ROUTE_NAME_MAX + 1];
    char mode[MODE_NAME_MAX + 1];
    uint32_t parent_index;
    uint32_t declared_nodes;
    Node *nodes;
    size_t node_count;
    size_t node_cap;
    Segment *segments;
    size_t segment_count;
    size_t segment_cap;
} Route;

typedef struct RouteDraft {
    Route route;
    int sealed;
} RouteDraft;

void route_draft_init(RouteDraft *draft);
void route_draft_free(RouteDraft *draft);
void route_set_header(RouteDraft *draft,
                      uint32_t id,
                      const char *label,
                      const char *mode,
                      uint32_t parent_index,
                      uint32_t declared_nodes);
Status route_add_node(RouteDraft *draft, uint32_t id, const char *name, uint32_t weight);
Status route_add_segment(RouteDraft *draft, uint32_t from, uint32_t to, uint32_t cost, const char *label);
Route *route_draft_seal(RouteDraft *draft);
void route_free(Route *route);
Node *route_node_at(Route *route, uint32_t index);
const Node *route_node_by_id(const Route *route, uint32_t id);
void route_print_summary(const Route *route);

#endif
