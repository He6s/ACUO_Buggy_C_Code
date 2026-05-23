#include "plugin.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef union CallbackBridge {
    PluginHandler plugin_handler;
    DispatchCallback dispatch_handler;
} CallbackBridge;

static void copy_text(char *dst, size_t cap, const char *src, const char *field) {
    size_t len = strlen(src);
    if (len >= cap) {
        die("plugin %s too long", field);
    }
    memcpy(dst, src, len + 1U);
}

void plugin_specs_init(PluginSpecList *list) {
    list->items = NULL;
    list->count = 0U;
    list->capacity = 0U;
}

void plugin_specs_free(PluginSpecList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0U;
    list->capacity = 0U;
}

void plugin_specs_add(PluginSpecList *list, const char *name, const char *topic, unsigned int flags, int use_filter) {
    if (list->count == list->capacity) {
        size_t next_capacity = list->capacity == 0U ? 4U : list->capacity * 2U;
        PluginSpec *next = realloc(list->items, next_capacity * sizeof(*next));
        if (next == NULL) {
            die("plugin spec allocation failed");
        }
        list->items = next;
        list->capacity = next_capacity;
    }
    PluginSpec *spec = &list->items[list->count];
    memset(spec, 0, sizeof(*spec));
    copy_text(spec->name, sizeof(spec->name), name, "name");
    copy_text(spec->topic, sizeof(spec->topic), topic, "topic");
    spec->flags = flags;
    spec->use_filter = use_filter;
    list->count++;
}

PluginContext *plugin_context_create(const char *name, const RuleBook *rules, unsigned int id, int fail_closed) {
    PluginContext *context = xcalloc(1U, sizeof(*context));
    PluginProfile *profile = xcalloc(1U, sizeof(*profile));
    copy_text(profile->name, sizeof(profile->name), name, "profile name");
    profile->rule_count = rules->count;
    profile->rules = xcalloc(profile->rule_count == 0U ? 1U : profile->rule_count, sizeof(*profile->rules));
    for (size_t i = 0U; i < profile->rule_count; i++) {
        profile->rules[i] = rules->items[i];
    }
    profile->default_flags = 0U;
    profile->fail_closed = fail_closed;
    context->profile = profile;
    context->plugin_id = id;
    context->last_topic = NULL;
    context->match_count = 0U;
    return context;
}

void plugin_context_destroy(PluginContext *context) {
    if (context == NULL) {
        return;
    }
    if (context->profile != NULL) {
        free(context->profile->rules);
        free(context->profile);
    }
    free(context);
}

DispatchCallback plugin_export_filter_callback(PluginHandler handler) {
    CallbackBridge bridge;
    bridge.plugin_handler = handler;
    return bridge.dispatch_handler;
}

static int rule_matches_message(const Rule *rule, const Message *message, unsigned int flags) {
    if (strcmp(rule->topic, message->topic) != 0) {
        return 0;
    }
    if ((flags & rule->required_flags) != rule->required_flags) {
        return 0;
    }
    return 1;
}

static int apply_rule(const Rule *rule, const Message *message) {
    switch (rule->action) {
        case RULE_ALLOW:
            printf("allowed seq=%u topic=%s tag=%s\n", message->sequence, message->topic, rule->tag);
            return 0;
        case RULE_DROP:
            printf("dropped seq=%u topic=%s tag=%s\n", message->sequence, message->topic, rule->tag);
            return 1;
        case RULE_AUDIT:
            printf("audited seq=%u topic=%s tag=%s payload=%s\n",
                   message->sequence,
                   message->topic,
                   rule->tag,
                   message->payload);
            return 0;
        default:
            return 1;
    }
}

int plugin_filter_handler(PluginContext *plugin, const Message *message, unsigned int flags) {
    PluginProfile *profile = plugin->profile;
    plugin->last_topic = message->topic;

    for (size_t i = 0U; i < profile->rule_count; i++) {
        const Rule *rule = &profile->rules[i];
        if (rule_matches_message(rule, message, flags)) {
            plugin->match_count++;
            return apply_rule(rule, message);
        }
    }

    if (profile->fail_closed) {
        printf("default drop seq=%u topic=%s profile=%s\n", message->sequence, message->topic, profile->name);
        return 1;
    }
    printf("default allow seq=%u topic=%s profile=%s\n", message->sequence, message->topic, profile->name);
    return 0;
}

static PluginContext *find_context(PluginContext **contexts, size_t context_count, const char *name) {
    for (size_t i = 0U; i < context_count; i++) {
        if (strcmp(contexts[i]->profile->name, name) == 0) {
            return contexts[i];
        }
    }
    return NULL;
}

void plugin_register_all(DispatchContext *dispatcher, PluginContext **contexts, size_t context_count, const PluginSpecList *specs) {
    for (size_t i = 0U; i < specs->count; i++) {
        const PluginSpec *spec = &specs->items[i];
        PluginContext *context = find_context(contexts, context_count, spec->name);
        if (context == NULL) {
            die("plugin spec references missing profile: %s", spec->name);
        }
        if (spec->use_filter) {
            DispatchCallback callback = plugin_export_filter_callback(plugin_filter_handler);
            dispatcher_subscribe(dispatcher, spec->name, spec->topic, callback, context, spec->flags);
        } else {
            dispatcher_subscribe(dispatcher, spec->name, spec->topic, dispatcher_audit_callback, context, spec->flags);
        }
    }
}
