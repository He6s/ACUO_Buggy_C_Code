#include <stdio.h>
#include <string.h>

#include "analyzer.h"
#include "catalog.h"
#include "parser.h"
#include "util.h"

static void usage(const char *prog)
{
    fprintf(stderr, "usage: %s trigger.txt [--trace]\n", prog);
}

static int has_flag(int argc, char **argv, const char *flag)
{
    int i;
    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], flag) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    ByteBuffer input;
    ParsedDoc doc;
    Catalog catalog;
    int trace;
    int status = 1;

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    trace = has_flag(argc, argv, "--trace");
    parser_init_doc(&doc);
    catalog_init(&catalog);

    if (util_load_file(argv[1], &input) != B3_OK) {
        goto out_no_input;
    }

    if (trace) {
        fprintf(stderr, "trace: loaded %zu bytes from %s\n", input.len, argv[1]);
        util_trace_bytes(stderr, "trace: file prefix", input.data, input.len, 96);
    }

    if (parser_parse_document(&input, &doc, trace) != B3_OK) {
        goto out;
    }
    if (catalog_build_from_doc(&catalog, &doc, trace) != B3_OK) {
        goto out;
    }
    if (trace) {
        catalog_dump(&catalog);
    }
    if (analyzer_run(&catalog, trace) != B3_OK) {
        goto out;
    }

    status = 0;

out:
    catalog_free(&catalog);
    parser_free_doc(&doc);
    util_free_buffer(&input);
out_no_input:
    return status;
}
