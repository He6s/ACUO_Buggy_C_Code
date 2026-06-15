#include "ilist.h"

#include "util.h"

#include <stddef.h>

void list_init(ListHead *list, const char *name) {
    list->head.prev = &list->head;
    list->head.next = &list->head;
    list->name = name;
    list->count = 0;
}

void list_node_init(ListNode *node) {
    node->prev = NULL;
    node->next = NULL;
}

bool list_node_linked(const ListNode *node) {
    return node->prev != NULL || node->next != NULL;
}

bool list_empty(const ListHead *list) {
    return list->head.next == &list->head;
}

void list_push_back(ListHead *list, ListNode *node) {
    ListNode *tail;

    if (list_node_linked(node)) {
        die("node already linked into a list");
    }
    tail = list->head.prev;
    node->prev = tail;
    node->next = &list->head;
    tail->next = node;
    list->head.prev = node;
    list->count++;
}

void list_remove(ListHead *list, ListNode *node) {
    ListNode *prev = node->prev;
    ListNode *next = node->next;

    if (prev == NULL || next == NULL) {
        die("attempted to remove unlinked node from %s", list->name);
    }

    /* The benchmark's visible crash normally happens on one of these writes. */
    prev->next = next;
    next->prev = prev;
    node->prev = NULL;
    node->next = NULL;
    if (list->count == 0) {
        die("list count underflow in %s", list->name);
    }
    list->count--;
}

ListNode *list_first(ListHead *list) {
    if (list_empty(list)) {
        return NULL;
    }
    return list->head.next;
}

void list_validate(const ListHead *list) {
    const ListNode *cur = list->head.next;
    const ListNode *prev = &list->head;
    size_t seen = 0;

    while (cur != &list->head) {
        if (cur == NULL) {
            die("null node encountered in %s", list->name);
        }
        if (cur->prev != prev) {
            die("broken prev link in %s", list->name);
        }
        prev = cur;
        cur = cur->next;
        seen++;
        if (seen > list->count + 8) {
            die("cycle or corrupted count in %s", list->name);
        }
    }
    if (seen != list->count) {
        die("list %s count mismatch: saw %zu expected %zu", list->name, seen, list->count);
    }
}
