#include "parser.h"

#include "util.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool key_is_valid(const char *key)
{
    size_t i;
    size_t len = strlen(key);

    if (len == 0U || len >= KEY_MAX) {
        return false;
    }
    for (i = 0U; i < len; i++) {
        unsigned char c = (unsigned char)key[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

static char *next_token(char **cursor)
{
    char *start;
    char *p;

    trim_left_in_place(cursor);
    if (**cursor == '\0') {
        return NULL;
    }

    start = *cursor;
    p = start;
    while (*p != '\0' && !isspace((unsigned char)*p)) {
        p++;
    }
    if (*p != '\0') {
        *p = '\0';
        p++;
    }
    *cursor = p;
    return start;
}

static void set_error(char *err, size_t err_cap, const char *message)
{
    if (!safe_copy(err, err_cap, message)) {
        die("internal error buffer too small");
    }
}

void command_init(Command *cmd, unsigned line_no)
{
    cmd->type = CMD_NONE;
    cmd->key[0] = '\0';
    cmd->value[0] = '\0';
    cmd->count = 0U;
    cmd->fill = '\0';
    cmd->line_no = line_no;
}

bool parse_command(char *line, unsigned line_no, Command *cmd, char *err, size_t err_cap)
{
    char *cursor = line;
    char *op;
    char *key;
    char *count_text;
    char *fill_text;

    command_init(cmd, line_no);
    strip_newline(line);
    trim_right_in_place(line);

    if (is_blank_or_comment(line)) {
        return true;
    }

    op = next_token(&cursor);
    if (op == NULL) {
        return true;
    }

    if (strcmp(op, "ADD") == 0) {
        key = next_token(&cursor);
        if (key == NULL || !key_is_valid(key)) {
            set_error(err, err_cap, "ADD requires a valid key");
            return false;
        }
        trim_left_in_place(&cursor);
        if (*cursor == '\0') {
            set_error(err, err_cap, "ADD requires a value");
            return false;
        }
        if (!safe_copy(cmd->key, sizeof(cmd->key), key)) {
            set_error(err, err_cap, "key is too long");
            return false;
        }
        if (!safe_copy(cmd->value, sizeof(cmd->value), cursor)) {
            set_error(err, err_cap, "value is too long");
            return false;
        }
        cmd->type = CMD_ADD;
        return true;
    }

    if (strcmp(op, "GET") == 0) {
        key = next_token(&cursor);
        if (key == NULL || !key_is_valid(key)) {
            set_error(err, err_cap, "GET requires a valid key");
            return false;
        }
        trim_left_in_place(&cursor);
        if (*cursor != '\0') {
            set_error(err, err_cap, "GET accepts only one key");
            return false;
        }
        if (!safe_copy(cmd->key, sizeof(cmd->key), key)) {
            set_error(err, err_cap, "key is too long");
            return false;
        }
        cmd->type = CMD_GET;
        return true;
    }

    if (strcmp(op, "FILL") == 0) {
        count_text = next_token(&cursor);
        fill_text = next_token(&cursor);
        if (count_text == NULL || !parse_size_arg(count_text, &cmd->count)) {
            set_error(err, err_cap, "FILL requires a byte count");
            return false;
        }
        if (fill_text == NULL || strlen(fill_text) != 1U) {
            set_error(err, err_cap, "FILL requires a single fill character");
            return false;
        }
        trim_left_in_place(&cursor);
        if (*cursor != '\0') {
            set_error(err, err_cap, "FILL accepts only count and character");
            return false;
        }
        cmd->fill = fill_text[0];
        cmd->type = CMD_FILL;
        return true;
    }

    if (strcmp(op, "DUMP") == 0) {
        cmd->type = CMD_DUMP;
        return true;
    }

    if (strcmp(op, "STATS") == 0) {
        cmd->type = CMD_STATS;
        return true;
    }

    if (strcmp(op, "EXIT") == 0) {
        cmd->type = CMD_EXIT;
        return true;
    }

    set_error(err, err_cap, "unknown command");
    return false;
}

const char *command_name(CommandType type)
{
    switch (type) {
    case CMD_NONE:
        return "NONE";
    case CMD_ADD:
        return "ADD";
    case CMD_GET:
        return "GET";
    case CMD_FILL:
        return "FILL";
    case CMD_DUMP:
        return "DUMP";
    case CMD_STATS:
        return "STATS";
    case CMD_EXIT:
        return "EXIT";
    default:
        return "UNKNOWN";
    }
}
