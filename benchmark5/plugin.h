#ifndef PLUGIN_H
#define PLUGIN_H

#include "dispatcher.h"
#include "message.h"
#include "rules.h"

#include <stddef.h>

#define PLUGIN_NAME_CAP 40

typedef struct PluginProfile {
    char name[PLUGIN_NAME_CAP];
    Rule *rules;
    size_t rule_count;
    unsigned int default_flags;
    int fail_closed;
} PluginProfile;

typedef struct PluginContext {
    PluginProfile *profile;
    unsigned int plugin_id;
    const char *last_topic;
    unsigned int match_count;
} PluginContext;

typedef int (*PluginHandler)(PluginContext *plugin, const Message *message, unsigned int flags);

typedef struct PluginSpec {
    char name[PLUGIN_NAME_CAP];
    char topic[48];
    unsigned int flags;
    int use_filter;
} PluginSpec;

typedef struct PluginSpecList {
    PluginSpec *items;
    size_t count;
    size_t capacity;
} PluginSpecList;

void plugin_specs_init(PluginSpecList *list);
void plugin_specs_free(PluginSpecList *list);
void plugin_specs_add(PluginSpecList *list, const char *name, const char *topic, unsigned int flags, int use_filter);
PluginContext *plugin_context_create(const char *name, const RuleBook *rules, unsigned int id, int fail_closed);
void plugin_context_destroy(PluginContext *context);
void plugin_register_all(DispatchContext *dispatcher, PluginContext **contexts, size_t context_count, const PluginSpecList *specs);
int plugin_filter_handler(PluginContext *plugin, const Message *message, unsigned int flags);
DispatchCallback plugin_export_filter_callback(PluginHandler handler);

#endif
