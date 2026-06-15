#ifndef TASK_H
#define TASK_H

#include "ilist.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum TaskState {
    TASK_WAITING = 0,
    TASK_READY = 1,
    TASK_DONE = 2
} TaskState;

typedef struct Task {
    uint32_t id;
    unsigned priority;
    TaskState state;
    char owner[16];
    char note[24];
    ListNode queue_link;
    ListNode audit_link;
    char title[32];
    unsigned retries;
} Task;

typedef struct TaskTable {
    Task **items;
    size_t count;
    size_t capacity;
} TaskTable;

void task_table_init(TaskTable *table);
void task_table_destroy(TaskTable *table);
Task *task_create(uint32_t id, const char *owner, unsigned priority, const char *title);
void task_table_add(TaskTable *table, Task *task);
Task *task_table_find(TaskTable *table, uint32_t id);
void task_set_note(Task *task, const char *bytes, size_t declared_len);
const char *task_state_name(TaskState state);
void task_print(const Task *task);

#endif
