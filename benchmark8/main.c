#include "parser.h"
#include "report.h"
#include "scheduler.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void usage(const char *argv0) {
    fprintf(stderr, "usage: %s trigger.txt [--trace]\n", argv0);
}

int main(int argc, char **argv) {
    const char *path;
    bool trace = false;
    CommandList commands;
    Scheduler sched;
    size_t i;

    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return 2;
    }
    path = argv[1];
    if (argc == 3) {
        if (strcmp(argv[2], "--trace") != 0) {
            usage(argv[0]);
            return 2;
        }
        trace = true;
    }

    command_list_init(&commands);
    parse_file(path, &commands, trace);
    scheduler_init(&sched, trace);

    for (i = 0; i < commands.count; i++) {
        scheduler_apply(&sched, &commands.items[i]);
    }
    report_final_state(&sched);

    scheduler_destroy(&sched);
    command_list_destroy(&commands);
    return 0;
}
