#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define PACKET_START_BYTE 0xAAU
#define PACKET_MAX_PAYLOAD_SIZE 64U

typedef struct
{
    uint8_t type;
    uint8_t length;
    uint8_t payload[PACKET_MAX_PAYLOAD_SIZE];
    uint8_t checksum;
} packet_t;

uint8_t packet_checksum(const packet_t *packet);

#endif /* PACKET_H */
