#include "rules.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fixed_copy(char *dst, size_t cap, const char *src, const char *field) {
    size_t len = strlen(src);
    if (len >= cap) {
        die("rule %s too long", field);
    }
    memcpy(dst, src, len + 1U);
}

void rulebook_init(RuleBook *book) {
    book->items = NULL;
    book->count = 0U;
    book->capacity = 0U;
}

void rulebook_free(RuleBook *book) {
    free(book->items);
    book->items = NULL;
    book->count = 0U;
    book->capacity = 0U;
}

void rulebook_add(RuleBook *book, const char *topic, const char *tag, RuleAction action, unsigned int flags) {
    if (book->count == book->capacity) {
        size_t next_capacity = book->capacity == 0U ? 8U : book->capacity * 2U;
        Rule *next = realloc(book->items, next_capacity * sizeof(*next));
        if (next == NULL) {
            die("rulebook allocation failed");
        }
        book->items = next;
        book->capacity = next_capacity;
    }
    Rule *rule = &book->items[book->count];
    memset(rule, 0, sizeof(*rule));
    fixed_copy(rule->topic, sizeof(rule->topic), topic, "topic");
    fixed_copy(rule->tag, sizeof(rule->tag), tag, "tag");
    rule->action = action;
    rule->required_flags = flags;
    book->count++;
}

const Rule *rulebook_find_topic(const RuleBook *book, const char *topic) {
    for (size_t i = 0U; i < book->count; i++) {
        if (strcmp(book->items[i].topic, topic) == 0) {
            return &book->items[i];
        }
    }
    return NULL;
}

RuleAction rule_action_from_text(const char *text) {
    if (strcmp(text, "allow") == 0) {
        return RULE_ALLOW;
    }
    if (strcmp(text, "drop") == 0) {
        return RULE_DROP;
    }
    if (strcmp(text, "audit") == 0) {
        return RULE_AUDIT;
    }
    die("unknown rule action: %s", text);
    return RULE_DROP;
}

const char *rule_action_name(RuleAction action) {
    switch (action) {
        case RULE_ALLOW:
            return "allow";
        case RULE_DROP:
            return "drop";
        case RULE_AUDIT:
            return "audit";
        default:
            return "unknown";
    }
}

void rulebook_dump(const RuleBook *book, int trace) {
    if (!trace) {
        return;
    }
    for (size_t i = 0U; i < book->count; i++) {
        printf("[trace] rule[%zu] topic=%s tag=%s action=%s flags=0x%x addr=%p\n",
               i,
               book->items[i].topic,
               book->items[i].tag,
               rule_action_name(book->items[i].action),
               book->items[i].required_flags,
               (const void *)&book->items[i]);
    }
}
