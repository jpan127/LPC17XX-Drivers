#pragma once 

// direction = 0
typedef enum
{
    PACKET_TYPE_INFO   = 0,  // System starting/finishing/initializing something, x bytes were sent
    PACKET_TYPE_ERROR  = 1,  // Something failed
    PACKET_TYPE_STATUS = 2,  // Something successful, something complete
} packet_diagnostic_type_E;

// direction = 1
typedef enum
{
    PACKET_TYPE_COMMAND_READ  = 0,  // Read commands to the decoder
    PACKET_TYPE_COMMAND_WRITE = 1,  // Write commands to the decoder
} packet_command_type_E;

// Opcode for command packets
typedef enum
{
    PACKET_OPCODE_NONE            = 0,
    PACKET_OPCODE_BASS            = 1,
    PACKET_OPCODE_TREBLE          = 2,
    PACKET_OPCODE_SAMPLE_RATE     = 3,
    PACKET_OPCODE_PLAY_CURRENT    = 4,
    PACKET_OPCODE_PLAY_NEXT       = 5,
    PACKET_OPCODE_PLAY_PREV       = 6,
    PACKET_OPCODE_STOP            = 7,
    PACKET_OPCODE_FAST_FORWARD    = 8,
    PACKET_OPCODE_REVERSE         = 9,
    PACKET_OPCODE_SHUFFLE         = 10,
    PACKET_OPCODE_GET_STATUS      = 11,
    PACKET_OPCODE_GET_SAMPLE_RATE = 12,
    PACKET_OPCODE_GET_DECODE_TIME = 13,
    PACKET_OPCODE_GET_HEADER_INFO = 14,
    PACKET_OPCODE_GET_BIT_RATE    = 15,
    PACKET_OPCODE_RESET           = 16,
    PACKET_OPCODE_LAST_INVALID    = 17,
} packet_opcode_E;

typedef enum
{
    HEADER1     = 0,
    HEADER2     = 1,
    PAYLOAD     = 2,
    COMMAND1    = 3,
    COMMAND2    = 4,
} parser_state_E;

typedef enum
{
    PARSER_IN_PROGRESS,
    PARSER_COMPLETE,
    PARSER_ERROR
} parser_status_E;

// Diagnostic Packet
typedef struct
{
    union
    {
        uint16_t value;
        struct
        {
            uint8_t length    : 8;  // Size of payload in bytes
            uint8_t opcode    : 5;  // Type of contents, only for commands
            uint8_t type      : 2;  // Type of packet
            uint8_t direction : 1;  // 0 for diagnostic, 1 for command
        } bits;
    } header;

    uint8_t payload[128];           // The actual data
} __attribute__((packed)) diagnostic_packet_S;

// Command Packet
typedef struct
{
    union
    {
        uint8_t value;
        struct
        {
            uint8_t opcode    : 5;  // Type of contents, only for commands
            uint8_t type      : 2;  // Type of packet
            uint8_t direction : 1;  // 0 for diagnostic, 1 for command
        } bits;
    } header;

    uint8_t command[2];             // The actual data
} __attribute__((packed)) command_packet_S;

// Queues
QueueHandle_t MessageRxQueue = xQueueCreate(3, sizeof(command_packet_S));
QueueHandle_t MessageTxQueue = xQueueCreate(3, sizeof(diagnostic_packet_S));

// @description : Converts an enum to a string
// @param state : The enum to be converted
// @returns     : A const char string literal
const char* parser_state_enum_to_string(parser_state_E state);

// @description  : State machine to parse a command packet
// @param byte   : The next byte to be parsed
// @param packet : Pointer to the packet to be modified
// @returns      : Status of parser state machine
parser_status_E command_packet_parser(uint8_t byte, command_packet_S *packet);

// @description  : State machine to parse a diagnostic packet
// @param byte   : The next byte to be parsed
// @param packet : Pointer to the packet to be modified
// @returns      : Status of parser state machine
parser_status_E diagnostic_packet_parser(uint8_t byte, diagnostic_packet_S *packet);

// @description    : Combines all the individual components into a diagnostic packet to be transmitted
// @param contents : Each param excluding the packet is a piece of the packet contents
// @param packet   : Pointer to the packet to be built
void create_diagnostic_packet(uint8_t direction, uint8_t type, uint8_t opcode, uint8_t length, uint8_t *buffer, diagnostic_packet_S *packet);