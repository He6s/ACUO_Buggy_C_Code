#include "parser.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static bool parse_unsigned(const char *text, unsigned *out_value) {
    char *end = NULL;
    unsigned long value;

    if (text == NULL || *text == '\0') {
        return false;
    }

    errno = 0;
    value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value > UINT_MAX) {
        return false;
    }

    *out_value = (unsigned)value;
    return true;
}

static bool expect_no_extra_tokens(char *cursor) {
    return next_token(&cursor) == NULL;
}

static bool parse_add(char *cursor, unsigned line_no, Command *out_cmd) {
    char *key;
    char *value;
    char *ttl_text;
    unsigned ttl = 0;

    (void)line_no;

    key = next_token(&cursor);
    value = next_token(&cursor);
    ttl_text = next_token(&cursor);

    if (key == NULL || value == NULL || ttl_text == NULL) {
        fprintf(stderr, "line %u: ADD requires key value ttl\n", line_no);
        return false;
    }

    if (!expect_no_extra_tokens(cursor)) {
        fprintf(stderr, "line %u: ADD received unexpected extra tokens\n", line_no);
        return false;
    }

    if (!parse_unsigned(ttl_text, &ttl)) {
        fprintf(stderr, "line %u: invalid ttl '%s'\n", line_no, ttl_text);
        return false;
    }

    out_cmd->type = CMD_ADD;
    out_cmd->record = record_create(key, strlen(key), value, strlen(value), ttl);
    return true;
}

static bool parse_find(char *cursor, unsigned line_no, Command *out_cmd) {
    char *key = next_token(&cursor);

    if (key == NULL) {
        fprintf(stderr, "line %u: FIND requires key\n", line_no);
        return false;
    }
    if (!expect_no_extra_tokens(cursor)) {
        fprintf(stderr, "line %u: FIND received unexpected extra tokens\n", line_no);
        return false;
    }

    out_cmd->type = CMD_FIND;
    out_cmd->lookup_key = page_strdup_len(key, strlen(key));
    out_cmd->lookup_key_len = strlen(key);
    return true;
}

static bool parse_delete(char *cursor, unsigned line_no, Command *out_cmd) {
    char *key = next_token(&cursor);

    if (key == NULL) {
        fprintf(stderr, "line %u: DEL requires key\n", line_no);
        return false;
    }
    if (!expect_no_extra_tokens(cursor)) {
        fprintf(stderr, "line %u: DEL received unexpected extra tokens\n", line_no);
        return false;
    }

    out_cmd->type = CMD_DEL;
    out_cmd->lookup_key = page_strdup_len(key, strlen(key));
    out_cmd->lookup_key_len = strlen(key);
    return true;
}

bool parser_read_command(FILE *fp, unsigned line_no, Command *out_cmd) {
    char buffer[512];
    char *cursor;
    char *verb;

    if (fp == NULL || out_cmd == NULL) {
        return false;
    }

    memset(out_cmd, 0, sizeof(*out_cmd));
    out_cmd->line_no = line_no;

    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        return false;
    }

    trim_newline(buffer);
    cursor = skip_ws(buffer);
    if (*cursor == '\0' || *cursor == '#') {
        out_cmd->type = CMD_HELP;
        return true;
    }

    verb = next_token(&cursor);
    lowercase_ascii(verb);

    if (strcmp(verb, "add") == 0) {
        return parse_add(cursor, line_no, out_cmd);
    }
    if (strcmp(verb, "find") == 0) {
        return parse_find(cursor, line_no, out_cmd);
    }
    if (strcmp(verb, "del") == 0) {
        return parse_delete(cursor, line_no, out_cmd);
    }
    if (strcmp(verb, "dump") == 0) {
        if (!expect_no_extra_tokens(cursor)) {
            fprintf(stderr, "line %u: DUMP takes no arguments\n", line_no);
            return false;
        }
        out_cmd->type = CMD_DUMP;
        return true;
    }
    if (strcmp(verb, "help") == 0) {
        out_cmd->type = CMD_HELP;
        return true;
    }

    fprintf(stderr, "line %u: unknown command '%s'\n", line_no, verb);
    return false;
}

void command_dispose(Command *cmd) {
    if (cmd == NULL) {
        return;
    }
    if (cmd->record != NULL) {
        record_destroy(cmd->record);
        cmd->record = NULL;
    }
    if (cmd->lookup_key != NULL) {
        page_strfree(cmd->lookup_key);
        cmd->lookup_key = NULL;
    }
    cmd->lookup_key_len = 0;
    cmd->type = CMD_INVALID;
    cmd->line_no = 0;
}

const char *command_type_name(CommandType type) {
    switch (type) {
        case CMD_INVALID:
            return "INVALID";
        case CMD_ADD:
            return "ADD";
        case CMD_FIND:
            return "FIND";
        case CMD_DEL:
            return "DEL";
        case CMD_DUMP:
            return "DUMP";
        case CMD_HELP:
            return "HELP";
    }
    return "?";
}
