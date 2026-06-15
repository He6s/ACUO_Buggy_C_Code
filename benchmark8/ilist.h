#ifndef ILIST_H
#define ILIST_H

#include <stdbool.h>
#include <stddef.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

typedef struct ListNode {
    struct ListNode *prev;
    struct ListNode *next;
} ListNode;

typedef struct ListHead {
    ListNode head;
    const char *name;
    size_t count;
} ListHead;

void list_init(ListHead *list, const char *name);
void list_node_init(ListNode *node);
bool list_node_linked(const ListNode *node);
bool list_empty(const ListHead *list);
void list_push_back(ListHead *list, ListNode *node);
void list_remove(ListHead *list, ListNode *node);
ListNode *list_first(ListHead *list);
void list_validate(const ListHead *list);

#endif
