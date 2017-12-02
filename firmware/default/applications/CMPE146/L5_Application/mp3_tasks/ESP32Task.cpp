#include "mp3_tasks.hpp"

#include <queue.h>
#include "uart.hpp"
#include "msg_protocol.hpp"

#define UART (Uart3::getInstance())

static const uint8_t end_of_packet_symbol = 0xAA;

void ESP32Task(void *p)
{
    // Setup uart
    const uint32_t baud_rate = 115200;
    UART.Init(baud_rate);

    diagnostic_packet_S diagnostic_packet = { 0 };
    command_packet_S command_packet       = { 0 };
    uint8_t buffer[130] = { 0 };

    parser_status_E status;

    // Main loop
    while (1)
    {
        // Check if pending messages to be received from ESP32
        if (UART.RxAvailable())
        {
            // Run state machine
            status = command_packet_parser(UART.ReceiveByte, &command_packet);
            // Check status of state machine
            switch (status)
            {
                case PARSER_IN_PROGRESS:
                    // Do nothing, keep going
                    break;
                case PARSER_COMPLETE:
                    // Send to receive queue
                    xQueueSend(MessageRxQueue, command_packet, portMAX_DELAY);
                    break;
                case PARSER_ERROR:
                    // Reset it 
                    memset(command_packet, 0, sizeof(command_packet_S));
                    break;
            }
        }

        // Check if pending messages to be sent to ESP32 (no wait)
        if (xQueueReceive(MessageTxQueue, diagnostic_packet, 0))
        {
            // Convert packet to array
            diagnostic_packet_to_array(buffer, &diagnostic_packet);
            // Send bytes to ESP32
            for (int i=0; i<(2 + diagnostic_packet.header.bits.length); i++)
            {
                UART.SendByte(buffer[i]);
            }
        }
    }
}