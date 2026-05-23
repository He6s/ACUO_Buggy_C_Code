#include "dispatcher.h"
#include "input.h"
#include "plugin.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

static int has_trace_flag(int argc, char **argv) {
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--trace") == 0) {
            return 1;
        }
    }
    return 0;
}

static void usage(const char *program) {
    fprintf(stderr, "usage: %s trigger.txt [--trace]\n", program);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    int trace = has_trace_flag(argc, argv);
    if (trace) {
        setvbuf(stdout, NULL, _IONBF, 0);
    }
    Config config;
    config_init(&config);
    config_load(&config, argv[1], trace);

    DispatchContext dispatcher;
    dispatcher_init(&dispatcher, trace);

    PluginContext *contexts[1];
    contexts[0] = plugin_context_create(config.profile_name, &config.rules, 7U, config.fail_closed);
    tracef(trace,
           "plugin context name=%s context=%p profile=%p rules=%zu",
           config.profile_name,
           (void *)contexts[0],
           (void *)contexts[0]->profile,
           contexts[0]->profile->rule_count);

    plugin_register_all(&dispatcher, contexts, 1U, &config.plugin_specs);
    int status = dispatcher_run(&dispatcher, &config.messages);

    plugin_context_destroy(contexts[0]);
    dispatcher_free(&dispatcher);
    config_free(&config);
    return status;
}
