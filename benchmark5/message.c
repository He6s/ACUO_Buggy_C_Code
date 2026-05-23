#include "message.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_fixed(char *dst, size_t cap, const char *src, const char *field) {
    size_t len = strlen(src);
    if (len >= cap) {
        die("%s too long", field);
    }
    memcpy(dst, src, len + 1U);
}

void message_list_init(MessageList *list) {
    list->items = NULL;
    list->count = 0U;
    list->capacity = 0U;
}

void message_list_free(MessageList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0U;
    list->capacity = 0U;
}

void message_list_push(MessageList *list, const Message *message) {
    if (list->count == list->capacity) {
        size_t next_capacity = list->capacity == 0U ? 8U : list->capacity * 2U;
        Message *next = realloc(list->items, next_capacity * sizeof(*next));
        if (next == NULL) {
            die("message list allocation failed");
        }
        list->items = next;
        list->capacity = next_capacity;
    }
    list->items[list->count] = *message;
    list->count++;
}

Message message_make(unsigned int sequence, const char *topic, const char *payload, unsigned int flags) {
    Message message;
    memset(&message, 0, sizeof(message));
    message.sequence = sequence;
    message.payload_len = (uint32_t)strlen(payload);
    copy_fixed(message.topic, sizeof(message.topic), topic, "topic");
    copy_fixed(message.payload, sizeof(message.payload), payload, "payload");
    message.flags = flags;
    return message;
}

void message_dump(const Message *message, int trace) {
    if (!trace) {
        return;
    }
    printf("[trace] message seq=%u payload_len=%u topic=%s flags=0x%x addr=%p\n",
           message->sequence,
           message->payload_len,
           message->topic,
           message->flags,
           (const void *)message);
}
