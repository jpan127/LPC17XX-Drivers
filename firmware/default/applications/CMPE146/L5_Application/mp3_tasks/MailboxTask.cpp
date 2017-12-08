#include "mp3_tasks.hpp"

#include "uart.hpp"
#include "msg_protocol.hpp"

#define UART (Uart3::getInstance())

///////////////////////////////////////////////////////////////////////
//    ESP32 --> UART --> ESP32Task --> MessageRxQueue --> MP3Task    //
//    MP3Task --> MessageTxQueue --> ESP32Task --> UART --> ESP32    //
///////////////////////////////////////////////////////////////////////

void MailboxTask(void *p)
{
    // Setup uart
    const uint32_t baud_rate = 115200;
    UART.Init(baud_rate);

    diagnostic_packet_S diagnostic_packet = { 0 };
    command_packet_S command_packet       = { 0 };
    uint8_t buffer[130] = { 0 };

    // Status flags
    parser_status_E status;
    bool sending = false;
    uint32_t rx_buffer_pointer = 0;

    // Main loop
    while (1)
    {
        // Check if pending messages to be received from ESP32
        if (UART.RxAvailable())
        {
            // Run state machine one byte at a time
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

        if (!sending)
        {
            // Check if pending messages to be sent to ESP32 (no wait)
            if (xQueueReceive(MessageTxQueue, &diagnostic_packet, 0))
            {
                sending = true;
                rx_buffer_pointer = 0;
                // Convert packet to array
                diagnostic_packet_to_array(buffer, &diagnostic_packet);
            }
        }
        else
        {
            // Send a byte to ESP32 at a time
            if (rx_buffer_pointer < diagnostic_packet.header.bits.length)
            {
                UART.SendByte(buffer[rx_buffer_pointer++]);
            }
            // Done
            else
            {
                sending = false;
            }
        }

        vTaskDelay(1 / periodTICK_PERIOD_MS);
    }
}