#include "packet.h"

// --- Encoding (outgoing) ---

static uint8_t Packet_Checksum(PacketMsgType type, const uint8_t *payload, uint8_t length)
{
    uint8_t checksum = type ^ length;
    for (uint8_t i = 0; i < length; i++) {
        checksum ^= payload[i];
    }
    return checksum;
}

uint8_t Packet_Encode(PacketMsgType type, const uint8_t *payload, uint8_t length, uint8_t *out_buf)
{
    if (length > PACKET_MAX_PAYLOAD) {
        return 0;
    }

    uint8_t idx = 0;
    out_buf[idx++] = PACKET_START_BYTE;
    out_buf[idx++] = (uint8_t)type;
    out_buf[idx++] = length;

    for (uint8_t i = 0; i < length; i++) {
        out_buf[idx++] = payload[i];
    }

    out_buf[idx++] = Packet_Checksum(type, payload, length);

    return idx;
}

// --- Parsing (incoming) — byte-at-a-time state machine ---

typedef enum {
    PSTATE_WAIT_START,
    PSTATE_WAIT_TYPE,
    PSTATE_WAIT_LENGTH,
    PSTATE_WAIT_PAYLOAD,
    PSTATE_WAIT_CHECKSUM,
} ParserState;

static ParserState state = PSTATE_WAIT_START;
static Packet in_progress;
static uint8_t payload_idx;

uint8_t Packet_ParseByte(uint8_t byte, Packet *out_packet)
{
    switch (state) {

        case PSTATE_WAIT_START:
            if (byte == PACKET_START_BYTE) {
                state = PSTATE_WAIT_TYPE;
            }
            break;

        case PSTATE_WAIT_TYPE:
            in_progress.type = (PacketMsgType)byte;
            state = PSTATE_WAIT_LENGTH;
            break;

        case PSTATE_WAIT_LENGTH:
            if (byte > PACKET_MAX_PAYLOAD) {
                state = PSTATE_WAIT_START;
                break;
            }
            in_progress.length = byte;
            payload_idx = 0;
            state = (byte == 0) ? PSTATE_WAIT_CHECKSUM : PSTATE_WAIT_PAYLOAD;
            break;

        case PSTATE_WAIT_PAYLOAD:
            in_progress.payload[payload_idx++] = byte;
            if (payload_idx >= in_progress.length) {
                state = PSTATE_WAIT_CHECKSUM;
            }
            break;

        case PSTATE_WAIT_CHECKSUM: {
            uint8_t expected = Packet_Checksum(in_progress.type, in_progress.payload, in_progress.length);
            state = PSTATE_WAIT_START;

            if (byte == expected) {
                *out_packet = in_progress;
                return 1;
            }
            break;
        }
    }

    return 0;
}