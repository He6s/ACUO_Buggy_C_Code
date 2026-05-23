#ifndef RULES_H
#define RULES_H

#include <stddef.h>

#define RULE_TOPIC_CAP 48
#define RULE_TAG_CAP 32

typedef enum RuleAction {
    RULE_ALLOW = 1,
    RULE_DROP = 2,
    RULE_AUDIT = 3
} RuleAction;

typedef struct Rule {
    char topic[RULE_TOPIC_CAP];
    char tag[RULE_TAG_CAP];
    RuleAction action;
    unsigned int required_flags;
} Rule;

typedef struct RuleBook {
    Rule *items;
    size_t count;
    size_t capacity;
} RuleBook;

void rulebook_init(RuleBook *book);
void rulebook_free(RuleBook *book);
void rulebook_add(RuleBook *book, const char *topic, const char *tag, RuleAction action, unsigned int flags);
const Rule *rulebook_find_topic(const RuleBook *book, const char *topic);
RuleAction rule_action_from_text(const char *text);
const char *rule_action_name(RuleAction action);
void rulebook_dump(const RuleBook *book, int trace);

#endif
