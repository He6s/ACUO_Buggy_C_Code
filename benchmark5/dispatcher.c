#include "dispatcher.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_text(char *dst, size_t cap, const char *src, const char *field) {
    size_t len = strlen(src);
    if (len >= cap) {
        die("subscription %s too long", field);
    }
    memcpy(dst, src, len + 1U);
}

void dispatcher_init(DispatchContext *context, int trace) {
    context->subscriptions = NULL;
    context->count = 0U;
    context->capacity = 0U;
    context->trace = trace;
    context->delivered = 0U;
    context->dropped = 0U;
}

void dispatcher_free(DispatchContext *context) {
    free(context->subscriptions);
    context->subscriptions = NULL;
    context->count = 0U;
    context->capacity = 0U;
}

void dispatcher_subscribe(DispatchContext *context,
                          const char *name,
                          const char *topic,
                          DispatchCallback callback,
                          void *owner,
                          unsigned int flags) {
    if (context->count == context->capacity) {
        size_t next_capacity = context->capacity == 0U ? 8U : context->capacity * 2U;
        Subscription *next = realloc(context->subscriptions, next_capacity * sizeof(*next));
        if (next == NULL) {
            die("subscription allocation failed");
        }
        context->subscriptions = next;
        context->capacity = next_capacity;
    }
    Subscription *sub = &context->subscriptions[context->count];
    memset(sub, 0, sizeof(*sub));
    copy_text(sub->name, sizeof(sub->name), name, "name");
    copy_text(sub->topic, sizeof(sub->topic), topic, "topic");
    sub->callback = callback;
    sub->owner = owner;
    sub->flags = flags;
    tracef(context->trace,
           "subscribed name=%s topic=%s owner=%p flags=0x%x",
           sub->name,
           sub->topic,
           sub->owner,
           sub->flags);
    context->count++;
}

static int topic_matches(const Subscription *sub, const Message *message) {
    return strcmp(sub->topic, "*") == 0 || strcmp(sub->topic, message->topic) == 0;
}

static int dispatch_one(DispatchContext *context, const Message *message) {
    int status = 0;
    message_dump(message, context->trace);
    for (size_t i = 0U; i < context->count; i++) {
        Subscription *sub = &context->subscriptions[i];
        if (!topic_matches(sub, message)) {
            continue;
        }
        tracef(context->trace,
               "calling subscription[%zu] name=%s with message=%p context=%p",
               i,
               sub->name,
               (const void *)message,
               (void *)context);
        int rc = sub->callback(message, context);
        if (rc == 0) {
            context->delivered++;
        } else {
            context->dropped++;
            status = rc;
        }
    }
    return status;
}

int dispatcher_run(DispatchContext *context, const MessageList *messages) {
    int status = 0;
    dispatcher_dump(context);
    for (size_t i = 0U; i < messages->count; i++) {
        int rc = dispatch_one(context, &messages->items[i]);
        if (rc != 0) {
            status = rc;
        }
    }
    return status;
}

int dispatcher_audit_callback(const Message *message, DispatchContext *context) {
    tracef(context->trace,
           "audit saw seq=%u topic=%s payload_len=%u",
           message->sequence,
           message->topic,
           message->payload_len);
    return 0;
}

void dispatcher_dump(const DispatchContext *context) {
    if (!context->trace) {
        return;
    }
    printf("[trace] dispatcher subscriptions=%zu delivered=%u dropped=%u context=%p\n",
           context->count,
           context->delivered,
           context->dropped,
           (const void *)context);
    for (size_t i = 0U; i < context->count; i++) {
        const Subscription *sub = &context->subscriptions[i];
        printf("[trace]   sub[%zu] name=%s topic=%s owner=%p flags=0x%x\n",
               i,
               sub->name,
               sub->topic,
               sub->owner,
               sub->flags);
    }
}
