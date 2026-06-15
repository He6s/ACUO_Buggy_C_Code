#include "task.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_short(char *dst, size_t cap, const char *src) {
    size_t n = strlen(src);
    if (n >= cap) {
        n = cap - 1;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

void task_table_init(TaskTable *table) {
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

void task_table_destroy(TaskTable *table) {
    size_t i;

    for (i = 0; i < table->count; i++) {
        free(table->items[i]);
    }
    free(table->items);
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

Task *task_create(uint32_t id, const char *owner, unsigned priority, const char *title) {
    Task *task = xcalloc(1, sizeof(*task));
    task->id = id;
    task->priority = priority;
    task->state = TASK_WAITING;
    copy_short(task->owner, sizeof(task->owner), owner);
    copy_short(task->title, sizeof(task->title), title);
    task->note[0] = '\0';
    task->retries = 0;
    list_node_init(&task->queue_link);
    list_node_init(&task->audit_link);
    return task;
}

void task_table_add(TaskTable *table, Task *task) {
    if (task_table_find(table, task->id) != NULL) {
        die("duplicate task id %u", task->id);
    }
    if (table->count == table->capacity) {
        size_t new_cap = table->capacity == 0 ? 8 : table->capacity * 2;
        Task **new_items = xcalloc(new_cap, sizeof(*new_items));
        if (table->items != NULL) {
            memcpy(new_items, table->items, table->count * sizeof(*new_items));
            free(table->items);
        }
        table->items = new_items;
        table->capacity = new_cap;
    }
    table->items[table->count++] = task;
}

Task *task_table_find(TaskTable *table, uint32_t id) {
    size_t i;

    for (i = 0; i < table->count; i++) {
        if (table->items[i]->id == id) {
            return table->items[i];
        }
    }
    return NULL;
}

void task_set_note(Task *task, const char *bytes, size_t declared_len) {
    /* BUG: declared_len is trusted even though note has a fixed 24 byte capacity.
       This corrupts the intrusive queue_link metadata that follows note. */
    memcpy(task->note, bytes, declared_len);
    if (declared_len < sizeof(task->note)) {
        task->note[declared_len] = '\0';
    } else {
        task->note[sizeof(task->note) - 1] = '\0';
    }
}

const char *task_state_name(TaskState state) {
    switch (state) {
        case TASK_WAITING:
            return "waiting";
        case TASK_READY:
            return "ready";
        case TASK_DONE:
            return "done";
        default:
            return "unknown";
    }
}

void task_print(const Task *task) {
    printf("task id=%u owner=%s pri=%u state=%s title=%s note=%s\n",
           task->id,
           task->owner,
           task->priority,
           task_state_name(task->state),
           task->title,
           task->note);
}
