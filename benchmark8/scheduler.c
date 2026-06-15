#include "scheduler.h"

#include "ilist.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static void trace_task_addr(const Scheduler *sched, const char *tag, const Task *task) {
    if (!sched->trace) {
        return;
    }
    fprintf(stderr,
            "[%s] task=%p id=%u note=%p queue_link=%p prev=%p next=%p\n",
            tag,
            (const void *)task,
            task->id,
            (const void *)task->note,
            (const void *)&task->queue_link,
            (const void *)task->queue_link.prev,
            (const void *)task->queue_link.next);
}

static void enqueue_waiting(Scheduler *sched, Task *task) {
    task->state = TASK_WAITING;
    list_push_back(&sched->waiting, &task->queue_link);
    trace_task_addr(sched, "waiting-add", task);
}

static void move_to_ready(Scheduler *sched, Task *task) {
    if (task->state == TASK_WAITING) {
        trace_task_addr(sched, "ready-before-remove", task);
        list_remove(&sched->waiting, &task->queue_link);
    } else if (task->state == TASK_READY) {
        return;
    } else {
        die("task %u cannot be made ready from state %s", task->id, task_state_name(task->state));
    }
    task->state = TASK_READY;
    list_push_back(&sched->ready, &task->queue_link);
    trace_task_addr(sched, "ready-after-add", task);
}

static Task *choose_ready(Scheduler *sched) {
    ListNode *node = sched->ready.head.next;
    Task *best = NULL;

    while (node != &sched->ready.head) {
        Task *candidate = container_of(node, Task, queue_link);
        if (best == NULL || candidate->priority > best->priority) {
            best = candidate;
        }
        node = node->next;
    }
    return best;
}

static void finish_task(Scheduler *sched, Task *task) {
    trace_task_addr(sched, "finish-before-remove", task);
    list_remove(&sched->ready, &task->queue_link);
    task->state = TASK_DONE;
    list_push_back(&sched->done, &task->queue_link);
    trace_task_addr(sched, "finish-after-add", task);
}

void scheduler_init(Scheduler *sched, bool trace) {
    task_table_init(&sched->tasks);
    list_init(&sched->waiting, "waiting");
    list_init(&sched->ready, "ready");
    list_init(&sched->done, "done");
    sched->trace = trace;
}

void scheduler_destroy(Scheduler *sched) {
    task_table_destroy(&sched->tasks);
}

void scheduler_apply(Scheduler *sched, const Command *cmd) {
    Task *task;

    switch (cmd->type) {
        case CMD_TASK:
            task = task_create(cmd->id, cmd->owner, cmd->priority, cmd->title);
            task_table_add(&sched->tasks, task);
            enqueue_waiting(sched, task);
            break;
        case CMD_READY:
            task = task_table_find(&sched->tasks, cmd->id);
            if (task == NULL) {
                die("line %u: unknown task id %u", cmd->line_no, cmd->id);
            }
            move_to_ready(sched, task);
            break;
        case CMD_NOTE:
            task = task_table_find(&sched->tasks, cmd->id);
            if (task == NULL) {
                die("line %u: unknown note task id %u", cmd->line_no, cmd->id);
            }
            trace_task_addr(sched, "note-before", task);
            task_set_note(task, cmd->note_text, cmd->note_len);
            trace_task_addr(sched, "note-after", task);
            break;
        case CMD_RUN:
            scheduler_run(sched);
            break;
        case CMD_DUMP:
            scheduler_dump(sched);
            break;
        default:
            die("unsupported command type");
    }
}

void scheduler_run(Scheduler *sched) {
    size_t turns = 0;

    while (!list_empty(&sched->ready)) {
        Task *task = choose_ready(sched);
        if (task == NULL) {
            break;
        }
        if (sched->trace) {
            fprintf(stderr, "[run] selected id=%u pri=%u\n", task->id, task->priority);
        }
        finish_task(sched, task);
        turns++;
        if (turns > sched->tasks.count + 4) {
            die("run loop exceeded expected number of tasks");
        }
    }
}

void scheduler_dump(Scheduler *sched) {
    size_t i;

    printf("tasks=%zu waiting=%zu ready=%zu done=%zu\n",
           sched->tasks.count,
           sched->waiting.count,
           sched->ready.count,
           sched->done.count);
    for (i = 0; i < sched->tasks.count; i++) {
        task_print(sched->tasks.items[i]);
    }
}
