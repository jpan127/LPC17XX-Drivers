#include "msg_protocol.hpp"

const char* parser_state_enum_to_string(parser_state_E state)
{
    static const char* names[] = {
        [HEADER1]  = "HEADER1",
        [HEADER2]  = "HEADER2",
        [PAYLOAD]  = "PAYLOAD",
        [COMMAND1] = "COMMAND1",
        [COMMAND2] = "COMMAND2",
    };

    return names[state];
}

void diagnostic_packet_to_array(uint8_t *array, diagnostic_packet_S *packet)
{
    array[0] = packet->header.value >> 8;
    array[1] = packet->header.value & 0xFF;
    memcpy(array + 2, packet->payload, packet->header.bits.length);
}

parser_status_E command_packet_parser(uint8_t byte, command_packet_S *packet)
{
    static parser_state_E state = HEADER1;

    switch (state)
    {
        case HEADER1:
            packet->header.value = byte;
            state = COMMAND1;
            return PARSER_IN_PROGRESS;
        case COMMAND1:
            packet->command[0] = byte;
            state = COMMAND2;
            return PARSER_IN_PROGRESS;
        case COMMAND2:
            packet->command[1] = byte;
            state = HEADER1;
            return PARSER_COMPLETE;
        default:
            printf("[command_packet_parser] Should never reach this state : %s!\n", parser_state_enum_to_string(state));
            state = HEADER1;
            return PARSER_ERROR;
    }
}

// Just realized this code should be on the ESP32!!
parser_status_E diagnostic_packet_parser(uint8_t byte, diagnostic_packet_S *packet)
{
    static parser_state_E state = HEADER1;
    static uint8_t counter = 0;

    switch (state)
    {
        case HEADER1:
            counter = 0;
            packet->header.value |= byte << 8;
            state = HEADER2;
            return PARSER_IN_PROGRESS;
        case HEADER2:
            packet->header.value |= byte;
            state = PAYLOAD;
            return PARSER_IN_PROGRESS;
        case PAYLOAD:
            // Check length to see if there's still bytes left to read in the payload
            if (counter < packet->header.bits.length)
            {
                packet->payload[counter++] = byte;
                // Don't change state
                return PARSER_IN_PROGRESS;
            }
            // Finished payload
            else
            {
                state = HEADER1;
                return PARSER_COMPLETE;
            }
        default:
            printf("[command_packet_parser] Should never reach this state : %s!\n", parser_state_enum_to_string(state));
            state = HEADER1;
            return PARSER_ERROR;
    }
}

void create_diagnostic_packet(uint8_t direction, uint8_t type, uint8_t opcode, uint8_t length, uint8_t *buffer, diagnostic_packet_S *packet)
{
    memset(packet, 0, sizeof(diagnostic_packet_S));
    packet->header.value = (length << 8) | ((opcode & 0x1F) << 3) | ((type & 0x3) << 1) | (direction & 0x1);
    memcpy(packet->payload, buffer, sizeof(uint8_t) * length);
}