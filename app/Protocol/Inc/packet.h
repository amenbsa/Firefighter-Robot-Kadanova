#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stddef.h>

#define PACKET_START_BYTE   0xAA
#define PACKET_START_BYTE   0xAA
#define PACKET_MAX_PAYLOAD  32
#define PACKET_MAX_FRAME    (PACKET_MAX_PAYLOAD + 4)

typedef enum {
    MSG_HEARTBEAT     = 0x01,
    MSG_SENSOR_DATA   = 0x02,
    MSG_FIRE_ALERT    = 0x03,
    MSG_MOTION_STATUS = 0x04,
} PacketMsgType;

typedef struct {
    PacketMsgType type;
    uint8_t       payload[PACKET_MAX_PAYLOAD];
    uint8_t       length;
} Packet;

uint8_t Packet_Encode(PacketMsgType type, const uint8_t *payload, uint8_t length, uint8_t *out_buf);
uint8_t Packet_ParseByte(uint8_t byte, Packet *out_packet);

#endif