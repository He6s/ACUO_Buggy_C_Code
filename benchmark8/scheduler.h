#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "parser.h"
#include "task.h"

#include <stdbool.h>

typedef struct Scheduler {
    TaskTable tasks;
    ListHead waiting;
    ListHead ready;
    ListHead done;
    bool trace;
} Scheduler;

void scheduler_init(Scheduler *sched, bool trace);
void scheduler_destroy(Scheduler *sched);
void scheduler_apply(Scheduler *sched, const Command *cmd);
void scheduler_run(Scheduler *sched);
void scheduler_dump(Scheduler *sched);

#endif
