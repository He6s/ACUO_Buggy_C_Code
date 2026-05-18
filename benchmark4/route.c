#include "route.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_fixed(char *dst, size_t cap, const char *src) {
    size_t len;
    if (cap == 0) {
        return;
    }
    len = strlen(src);
    if (len >= cap) {
        len = cap - 1;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

void route_draft_init(RouteDraft *draft) {
    memset(draft, 0, sizeof(*draft));
    draft->route.parent_index = NO_PARENT;
    draft->route.declared_nodes = 0;
    copy_fixed(draft->route.mode, sizeof(draft->route.mode), "direct");
}

void route_draft_free(RouteDraft *draft) {
    free(draft->route.nodes);
    free(draft->route.segments);
    memset(draft, 0, sizeof(*draft));
}

void route_set_header(RouteDraft *draft,
                      uint32_t id,
                      const char *label,
                      const char *mode,
                      uint32_t parent_index,
                      uint32_t declared_nodes) {
    draft->route.id = id;
    copy_fixed(draft->route.label, sizeof(draft->route.label), label);
    copy_fixed(draft->route.mode, sizeof(draft->route.mode), mode);
    draft->route.parent_index = parent_index;
    draft->route.declared_nodes = declared_nodes;
}

static void ensure_node_cap(RouteDraft *draft) {
    if (draft->route.node_count == draft->route.node_cap) {
        size_t next_cap = draft->route.node_cap == 0 ? 8 : draft->route.node_cap * 2;
        draft->route.nodes = xrealloc(draft->route.nodes, next_cap * sizeof(draft->route.nodes[0]));
        draft->route.node_cap = next_cap;
    }
}

static void ensure_segment_cap(RouteDraft *draft) {
    if (draft->route.segment_count == draft->route.segment_cap) {
        size_t next_cap = draft->route.segment_cap == 0 ? 8 : draft->route.segment_cap * 2;
        draft->route.segments = xrealloc(draft->route.segments, next_cap * sizeof(draft->route.segments[0]));
        draft->route.segment_cap = next_cap;
    }
}

static int node_id_exists(const RouteDraft *draft, uint32_t id) {
    size_t i;
    for (i = 0; i < draft->route.node_count; i++) {
        if (draft->route.nodes[i].id == id) {
            return 1;
        }
    }
    return 0;
}

Status route_add_node(RouteDraft *draft, uint32_t id, const char *name, uint32_t weight) {
    Node *node;
    if (draft->sealed) {
        return STATUS_ERR;
    }
    if (node_id_exists(draft, id)) {
        return STATUS_ERR;
    }
    ensure_node_cap(draft);
    node = &draft->route.nodes[draft->route.node_count++];
    node->id = id;
    node->weight = weight;
    copy_fixed(node->name, sizeof(node->name), name);
    return STATUS_OK;
}

Status route_add_segment(RouteDraft *draft, uint32_t from, uint32_t to, uint32_t cost, const char *label) {
    Segment *seg;
    if (draft->sealed) {
        return STATUS_ERR;
    }
    ensure_segment_cap(draft);
    seg = &draft->route.segments[draft->route.segment_count++];
    seg->from = from;
    seg->to = to;
    seg->cost = cost;
    copy_fixed(seg->label, sizeof(seg->label), label);
    return STATUS_OK;
}

Route *route_draft_seal(RouteDraft *draft) {
    Route *route;
    if (draft->sealed) {
        return NULL;
    }
    draft->sealed = 1;
    route = xcalloc(1, sizeof(*route));
    *route = draft->route;
    draft->route.nodes = NULL;
    draft->route.segments = NULL;
    draft->route.node_count = 0;
    draft->route.segment_count = 0;
    return route;
}

void route_free(Route *route) {
    if (route == NULL) {
        return;
    }
    free(route->nodes);
    free(route->segments);
    free(route);
}

Node *route_node_at(Route *route, uint32_t index) {
    return &route->nodes[index];
}

const Node *route_node_by_id(const Route *route, uint32_t id) {
    size_t i;
    for (i = 0; i < route->node_count; i++) {
        if (route->nodes[i].id == id) {
            return &route->nodes[i];
        }
    }
    return NULL;
}

void route_print_summary(const Route *route) {
    printf("route %u label=%s mode=%s nodes=%zu segments=%zu parent_index=%u declared_nodes=%u\n",
           route->id,
           route->label,
           route->mode,
           route->node_count,
           route->segment_count,
           route->parent_index,
           route->declared_nodes);
}
