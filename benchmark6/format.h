#ifndef FORMAT_H
#define FORMAT_H

#include <stdbool.h>
#include <stddef.h>

typedef struct Packet {
    char *data;
    size_t len;
    size_t cap;
    bool trace;
} Packet;

void packet_init(Packet *packet, bool trace);
void packet_destroy(Packet *packet);
void packet_add(Packet *packet, const char *spec, ...);
void packet_print(const Packet *packet);
const char *packet_data(const Packet *packet);
size_t packet_len(const Packet *packet);

#endif
