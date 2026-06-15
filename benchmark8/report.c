#include "report.h"

#include "ilist.h"
#include "task.h"

#include <stdio.h>

static void print_list(const char *name, ListHead *head) {
    ListNode *node = head->head.next;
    printf("%s:", name);
    while (node != &head->head) {
        Task *task = container_of(node, Task, queue_link);
        printf(" %u", task->id);
        node = node->next;
    }
    printf("\n");
}

void report_final_state(Scheduler *sched) {
    printf("final queues\n");
    print_list("waiting", &sched->waiting);
    print_list("ready", &sched->ready);
    print_list("done", &sched->done);
}
