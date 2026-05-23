#include "input.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_CAP 512
#define MAX_TOKENS 10

static void copy_profile_name(Config *config, const char *name) {
    size_t len = strlen(name);
    if (len >= sizeof(config->profile_name)) {
        die("profile name too long: %s", name);
    }
    memcpy(config->profile_name, name, len + 1U);
    config->has_profile = 1;
}

static size_t split_tokens(char *line, char **tokens, size_t cap) {
    size_t count = 0U;
    char *cursor = line;
    while (*cursor != '\0') {
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
        if (*cursor == '\0' || *cursor == '#') {
            break;
        }
        if (count == cap) {
            die("too many tokens in line: %s", line);
        }
        tokens[count] = cursor;
        count++;
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
            cursor++;
        }
        if (*cursor != '\0') {
            *cursor = '\0';
            cursor++;
        }
    }
    return count;
}

static unsigned int parse_required_u32(const char *text, const char *field) {
    unsigned int value = 0U;
    if (!parse_u32(text, &value)) {
        die("invalid %s: %s", field, text);
    }
    return value;
}

static void parse_profile(Config *config, char **tokens, size_t count) {
    if (count < 2U || count > 3U) {
        die("profile expects: profile NAME [fail_closed|fail_open]");
    }
    copy_profile_name(config, tokens[1]);
    config->fail_closed = 0;
    if (count == 3U) {
        if (strcmp(tokens[2], "fail_closed") == 0) {
            config->fail_closed = 1;
        } else if (strcmp(tokens[2], "fail_open") == 0) {
            config->fail_closed = 0;
        } else {
            die("unknown profile mode: %s", tokens[2]);
        }
    }
}

static void parse_rule(Config *config, char **tokens, size_t count) {
    if (count != 5U) {
        die("rule expects: rule TOPIC ACTION TAG FLAGS");
    }
    RuleAction action = rule_action_from_text(tokens[2]);
    unsigned int flags = parse_required_u32(tokens[4], "rule flags");
    rulebook_add(&config->rules, tokens[1], tokens[3], action, flags);
}

static int parse_filter_kind(const char *text) {
    if (strcmp(text, "filter") == 0) {
        return 1;
    }
    if (strcmp(text, "audit") == 0) {
        return 0;
    }
    die("unknown subscription kind: %s", text);
    return 0;
}

static void parse_subscribe(Config *config, char **tokens, size_t count) {
    if (count != 5U) {
        die("subscribe expects: subscribe PROFILE TOPIC KIND FLAGS");
    }
    unsigned int flags = parse_required_u32(tokens[4], "subscription flags");
    int use_filter = parse_filter_kind(tokens[3]);
    plugin_specs_add(&config->plugin_specs, tokens[1], tokens[2], flags, use_filter);
}

static void parse_message(Config *config, char **tokens, size_t count) {
    if (count != 5U) {
        die("message expects: message SEQUENCE TOPIC PAYLOAD FLAGS");
    }
    unsigned int sequence = parse_required_u32(tokens[1], "message sequence");
    unsigned int flags = parse_required_u32(tokens[4], "message flags");
    Message message = message_make(sequence, tokens[2], tokens[3], flags);
    message_list_push(&config->messages, &message);
}

void config_init(Config *config) {
    memset(config->profile_name, 0, sizeof(config->profile_name));
    config->has_profile = 0;
    config->fail_closed = 0;
    rulebook_init(&config->rules);
    plugin_specs_init(&config->plugin_specs);
    message_list_init(&config->messages);
}

void config_free(Config *config) {
    rulebook_free(&config->rules);
    plugin_specs_free(&config->plugin_specs);
    message_list_free(&config->messages);
}

void config_load(Config *config, const char *path, int trace) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        die("could not open %s", path);
    }

    char line[LINE_CAP];
    unsigned int line_no = 0U;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;
        trim_line(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        char copy[LINE_CAP];
        memcpy(copy, line, strlen(line) + 1U);
        char *tokens[MAX_TOKENS];
        size_t count = split_tokens(copy, tokens, MAX_TOKENS);
        if (count == 0U) {
            continue;
        }
        tracef(trace, "parse line %u kind=%s", line_no, tokens[0]);
        if (strcmp(tokens[0], "profile") == 0) {
            parse_profile(config, tokens, count);
        } else if (strcmp(tokens[0], "rule") == 0) {
            parse_rule(config, tokens, count);
        } else if (strcmp(tokens[0], "subscribe") == 0) {
            parse_subscribe(config, tokens, count);
        } else if (strcmp(tokens[0], "message") == 0) {
            parse_message(config, tokens, count);
        } else {
            fclose(fp);
            die("unknown directive on line %u: %s", line_no, tokens[0]);
        }
    }

    if (ferror(fp)) {
        fclose(fp);
        die("error while reading %s", path);
    }
    fclose(fp);

    if (!config->has_profile) {
        die("configuration missing profile");
    }
    if (config->messages.count == 0U) {
        die("configuration contains no messages");
    }
    if (config->plugin_specs.count == 0U) {
        die("configuration contains no subscriptions");
    }
    rulebook_dump(&config->rules, trace);
}
