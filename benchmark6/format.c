#include "format.h"

#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void packet_reserve(Packet *packet, size_t extra)
{
    size_t need = packet->len + extra + 1U;
    if (need > packet->cap) {
        size_t next = packet->cap == 0U ? 128U : packet->cap;
        while (next < need) {
            next *= 2U;
        }
        packet->data = xrealloc(packet->data, next);
        packet->cap = next;
    }
}

static void packet_append_raw(Packet *packet, const char *data, size_t len)
{
    packet_reserve(packet, len);
    memcpy(packet->data + packet->len, data, len);
    packet->len += len;
    packet->data[packet->len] = '\0';
}

static void packet_append_cstr(Packet *packet, const char *text)
{
    size_t len;

    /*
     * The benchmark usually crashes here when an integer vararg was consumed
     * as a pointer by packet_add. This site is intentionally downstream from
     * the incorrect call site in router.c.
     */
    len = strlen(text);
    packet_append_raw(packet, text, len);
}

static void packet_append_uint(Packet *packet, unsigned int value)
{
    char tmp[32];
    int written = snprintf(tmp, sizeof(tmp), "%u", value);
    if (written < 0) {
        die("snprintf failed");
    }
    packet_append_raw(packet, tmp, (size_t)written);
}

static void packet_append_bool(Packet *packet, int value)
{
    if (value != 0) {
        packet_append_raw(packet, "true", 4U);
    } else {
        packet_append_raw(packet, "false", 5U);
    }
}

void packet_init(Packet *packet, bool trace)
{
    packet->data = NULL;
    packet->len = 0U;
    packet->cap = 0U;
    packet->trace = trace;
    packet_reserve(packet, 0U);
    packet->data[0] = '\0';
}

void packet_destroy(Packet *packet)
{
    free(packet->data);
    packet->data = NULL;
    packet->len = 0U;
    packet->cap = 0U;
}

void packet_add(Packet *packet, const char *spec, ...)
{
    va_list ap;
    size_t i;

    va_start(ap, spec);
    for (i = 0U; spec[i] != '\0'; i++) {
        char token = spec[i];
        if (packet->trace) {
            printf("trace: packet_add token='%c' packet_len=%zu\n", token, packet->len);
        }
        if (token == 's') {
            const char *text = va_arg(ap, const char *);
            if (packet->trace) {
                printf("trace:   va_arg as string pointer=%p\n", (const void *)text);
            }
            packet_append_cstr(packet, text);
        } else if (token == 'u') {
            unsigned int value = va_arg(ap, unsigned int);
            if (packet->trace) {
                printf("trace:   va_arg as unsigned=%u\n", value);
            }
            packet_append_uint(packet, value);
        } else if (token == 'b') {
            int value = va_arg(ap, int);
            if (packet->trace) {
                printf("trace:   va_arg as bool/int=%d\n", value);
            }
            packet_append_bool(packet, value);
        } else if (token == '|') {
            packet_append_raw(packet, "|", 1U);
        } else if (token == '=') {
            packet_append_raw(packet, "=", 1U);
        } else if (token == '\n') {
            packet_append_raw(packet, "\n", 1U);
        } else {
            die("unknown packet spec token");
        }
    }
    va_end(ap);
}

void packet_print(const Packet *packet)
{
    fwrite(packet->data, 1U, packet->len, stdout);
    if (packet->len == 0U || packet->data[packet->len - 1U] != '\n') {
        putchar('\n');
    }
}

const char *packet_data(const Packet *packet)
{
    return packet->data;
}

size_t packet_len(const Packet *packet)
{
    return packet->len;
}
