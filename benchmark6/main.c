#include "parser.h"
#include "router.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Options {
    const char *path;
    bool trace;
} Options;

static void usage(const char *argv0)
{
    fprintf(stderr, "usage: %s trigger.txt [--trace]\n", argv0);
}

static bool parse_args(int argc, char **argv, Options *opts)
{
    int i;

    opts->path = NULL;
    opts->trace = false;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--trace") == 0) {
            opts->trace = true;
        } else if (opts->path == NULL) {
            opts->path = argv[i];
        } else {
            return false;
        }
    }
    return opts->path != NULL;
}

static bool run_file(const Options *opts)
{
    FILE *fp;
    Router router;
    Command cmd;
    char line[MAX_LINE];
    char error[MAX_TEXT];
    unsigned int lineno = 0U;
    bool ok = true;

    fp = fopen(opts->path, "r");
    if (fp == NULL) {
        perror(opts->path);
        return false;
    }
    router_init(&router, opts->trace);
    command_init(&cmd);
    while (fgets(line, sizeof(line), fp) != NULL) {
        lineno++;
        if (opts->trace) {
            printf("trace: line %u: %s", lineno, line);
        }
        if (!parse_line(line, &cmd, error, sizeof(error))) {
            fprintf(stderr, "%s:%u: %s\n", opts->path, lineno, error);
            ok = false;
            break;
        }
        if (!router_apply(&router, &cmd)) {
            fprintf(stderr, "%s:%u: command failed\n", opts->path, lineno);
            ok = false;
            break;
        }
    }
    if (ferror(fp)) {
        perror(opts->path);
        ok = false;
    }
    command_destroy(&cmd);
    router_destroy(&router);
    fclose(fp);
    return ok;
}

int main(int argc, char **argv)
{
    Options opts;

    if (!parse_args(argc, argv, &opts)) {
        usage(argv[0]);
        return 2;
    }
    if (!run_file(&opts)) {
        return 1;
    }
    return 0;
}
