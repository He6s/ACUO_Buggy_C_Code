#ifndef MESSAGE_H
#define MESSAGE_H

#include <stddef.h>
#include <stdint.h>

#define MESSAGE_TOPIC_CAP 48
#define MESSAGE_PAYLOAD_CAP 160

typedef struct Message {
    uint32_t sequence;
    uint32_t payload_len;
    char topic[MESSAGE_TOPIC_CAP];
    char payload[MESSAGE_PAYLOAD_CAP];
    unsigned int flags;
} Message;

typedef struct MessageList {
    Message *items;
    size_t count;
    size_t capacity;
} MessageList;

void message_list_init(MessageList *list);
void message_list_free(MessageList *list);
void message_list_push(MessageList *list, const Message *message);
Message message_make(unsigned int sequence, const char *topic, const char *payload, unsigned int flags);
void message_dump(const Message *message, int trace);

#endif
