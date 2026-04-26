#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "store.h"
#include "util.h"

static void print_usage(const char *argv0) {
    fprintf(stderr,
            "Usage: %s <command-file> [--trace]\n"
            "\n"
            "Commands\n"
            "  ADD <key> <value> <ttl>\n"
            "  FIND <key>\n"
            "  DEL <key>\n"
            "  DUMP\n"
            "  HELP\n",
            argv0);
}

static void print_help_line(unsigned line_no) {
    printf("line %u: blank/comment/help line\n", line_no);
}

static void trace_command(const Command *cmd) {
    printf("trace: line=%u type=%s\n", cmd->line_no, command_type_name(cmd->type));
    if (cmd->record != NULL) {
        debug_dump_ptr(stdout, "  record.key", cmd->record->key);
        debug_dump_ptr(stdout, "  record.value", cmd->record->value);
    }
    if (cmd->lookup_key != NULL) {
        debug_dump_ptr(stdout, "  lookup_key", cmd->lookup_key);
    }
}

static bool execute_add(Store *store, const Command *cmd, bool trace) {
    bool ok;

    if (trace) {
        printf("line %u: insert key='%s' value='%s' ttl=%u\n",
               cmd->line_no,
               cmd->record->key,
               cmd->record->value,
               cmd->record->ttl);
    }

    ok = store_insert(store,
                      cmd->record->key,
                      cmd->record->key_len,
                      cmd->record->value,
                      cmd->record->value_len,
                      cmd->record->ttl);

    if (!ok) {
        fprintf(stderr, "line %u: insert failed\n", cmd->line_no);
        return false;
    }

    if (trace) {
        printf("line %u: insert complete, store size now %zu\n",
               cmd->line_no, store_size(store));
    }

    return true;
}

static bool execute_find(Store *store, const Command *cmd, bool trace) {
    const char *value;

    if (trace) {
        printf("line %u: find key='%s'\n", cmd->line_no, cmd->lookup_key);
    }

    value = store_find(store, cmd->lookup_key, cmd->lookup_key_len);
    if (value == NULL) {
        printf("line %u: NOT FOUND\n", cmd->line_no);
        return true;
    }

    printf("line %u: FOUND '%s'\n", cmd->line_no, value);
    return true;
}

static bool execute_delete(Store *store, const Command *cmd, bool trace) {
    bool removed;

    if (trace) {
        printf("line %u: delete key='%s'\n", cmd->line_no, cmd->lookup_key);
    }

    removed = store_delete(store, cmd->lookup_key, cmd->lookup_key_len);
    if (removed) {
        printf("line %u: DELETED\n", cmd->line_no);
    } else {
        printf("line %u: NOT FOUND\n", cmd->line_no);
    }
    return true;
}

static bool execute_dump(Store *store, const Command *cmd) {
    (void)cmd;
    store_dump(store, stdout);
    return true;
}

static bool execute_command(Store *store, const Command *cmd, bool trace) {
    switch (cmd->type) {
        case CMD_ADD:
            return execute_add(store, cmd, trace);
        case CMD_FIND:
            return execute_find(store, cmd, trace);
        case CMD_DEL:
            return execute_delete(store, cmd, trace);
        case CMD_DUMP:
            return execute_dump(store, cmd);
        case CMD_HELP:
            print_help_line(cmd->line_no);
            return true;
        case CMD_INVALID:
            return false;
    }
    return false;
}

static bool parse_args(int argc, char **argv, const char **command_path, bool *trace) {
    int i;

    *command_path = NULL;
    *trace = false;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--trace") == 0) {
            *trace = true;
            continue;
        }
        if (*command_path == NULL) {
            *command_path = argv[i];
            continue;
        }
        return false;
    }

    return *command_path != NULL;
}

int main(int argc, char **argv) {
    const char *command_path;

    setvbuf(stdout, NULL, _IONBF, 0);
    bool trace;
    FILE *fp;
    Store *store;
    unsigned line_no = 1;
    bool ok = true;

    if (!parse_args(argc, argv, &command_path, &trace)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    fp = fopen(command_path, "r");
    if (fp == NULL) {
        perror(command_path);
        return EXIT_FAILURE;
    }

    store = store_create(4);

    for (;;) {
        Command cmd;
        bool has_line = parser_read_command(fp, line_no, &cmd);

        if (!has_line) {
            break;
        }

        if (trace) {
            trace_command(&cmd);
        }

        if (!execute_command(store, &cmd, trace)) {
            fprintf(stderr, "line %u: command execution failed\n", line_no);
            command_dispose(&cmd);
            ok = false;
            break;
        }

        command_dispose(&cmd);
        line_no++;
    }

    fclose(fp);
    store_destroy(store);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
