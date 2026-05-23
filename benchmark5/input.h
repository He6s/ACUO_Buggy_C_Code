#ifndef INPUT_H
#define INPUT_H

#include "message.h"
#include "plugin.h"
#include "rules.h"

#define CONFIG_PROFILE_CAP 40

typedef struct Config {
    char profile_name[CONFIG_PROFILE_CAP];
    int has_profile;
    int fail_closed;
    RuleBook rules;
    PluginSpecList plugin_specs;
    MessageList messages;
} Config;

void config_init(Config *config);
void config_free(Config *config);
void config_load(Config *config, const char *path, int trace);

#endif
