#include "packet.h"

uint8_t packet_checksum(const packet_t *packet)
{
    uint8_t checksum = packet->type ^ packet->length;

    for (uint8_t index = 0U; index < packet->length; ++index)
    {
        checksum ^= packet->payload[index];
    }

    return checksum;
}
