#include "database.h"
#include "parser.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *fp)
{
    fprintf(fp, "usage: %s <input-file> [--trace]\n", PROGRAM_NAME);
    fprintf(fp, "commands: ADD key value | GET key | FILL count char | DUMP | STATS | EXIT\n");
}

static bool has_flag(int argc, char **argv, const char *flag)
{
    int i;
    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], flag) == 0) {
            return true;
        }
    }
    return false;
}

static void validate_args(int argc, char **argv)
{
    int i;

    if (argc < 2) {
        usage(stderr);
        exit(2);
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        usage(stdout);
        exit(0);
    }
    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--trace") != 0) {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            usage(stderr);
            exit(2);
        }
    }
}

static FILE *open_input(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        die("could not open input file '%s'", path);
    }
    return fp;
}

static void run_file(Database *db, FILE *fp)
{
    char *line = NULL;
    size_t cap = 0U;
    size_t len = 0U;
    unsigned line_no = 0U;
    bool keep_running = true;

    while (keep_running && read_line(fp, &line, &cap, &len)) {
        Command cmd;
        char err[128];
        (void)len;
        line_no++;
        err[0] = '\0';
        if (!parse_command(line, line_no, &cmd, err, sizeof(err))) {
            free(line);
            die("parse error on line %u: %s", line_no, err);
        }
        keep_running = database_execute(db, &cmd);
    }

    free(line);
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    FILE *fp;
    Database db;
    bool trace;

    validate_args(argc, argv);
    trace = has_flag(argc, argv, "--trace");
    fp = open_input(argv[1]);

    database_init(&db, trace);
    run_file(&db, fp);

    if (fclose(fp) != 0) {
        database_destroy(&db);
        die("could not close input file");
    }
    database_destroy(&db);
    return 0;
}
