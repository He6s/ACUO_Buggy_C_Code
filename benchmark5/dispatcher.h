#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "message.h"

#include <stddef.h>

struct DispatchContext;

typedef int (*DispatchCallback)(const Message *message, struct DispatchContext *context);

typedef struct Subscription {
    char name[40];
    char topic[48];
    DispatchCallback callback;
    void *owner;
    unsigned int flags;
} Subscription;

typedef struct DispatchContext {
    Subscription *subscriptions;
    size_t count;
    size_t capacity;
    int trace;
    unsigned int delivered;
    unsigned int dropped;
} DispatchContext;

void dispatcher_init(DispatchContext *context, int trace);
void dispatcher_free(DispatchContext *context);
void dispatcher_subscribe(DispatchContext *context,
                          const char *name,
                          const char *topic,
                          DispatchCallback callback,
                          void *owner,
                          unsigned int flags);
int dispatcher_run(DispatchContext *context, const MessageList *messages);
int dispatcher_audit_callback(const Message *message, DispatchContext *context);
void dispatcher_dump(const DispatchContext *context);

#endif
