#ifndef PARSER_H
#define PARSER_H

#include "route.h"
#include "util.h"

#include <stddef.h>

typedef struct ParseOptions {
    int trace;
} ParseOptions;

Status parse_route_file(const char *path, const ParseOptions *options, Route **out_route);

#endif
