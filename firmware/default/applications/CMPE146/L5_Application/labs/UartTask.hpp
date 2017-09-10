#pragma once
#include <scheduler_task.hpp>
#include <string.h>
#include "L5_Application/drivers/uart.hpp"
#include "L5_Application/drivers/buttons.hpp"

#define UART_PORT   (UART_PORT3)
#define BUFFER_SIZE (1024)

// State of uart, writing or reading
typedef enum { TRANSMITTING, RECEIVING } uart_state_t;

class UartTask : public scheduler_task, public Uart
{
public:

    // Constructor
    UartTask(uint8_t priority, uart_port_t port);

    // Destructor
    ~UartTask();

    bool run(void *p);

private:

    Button          B0;         // Set state to TRANSMITTING
    Button          B1;         // Set state to RECEIVING
    Button          B2;         // Send message in TRANSMITTING state
    uart_state_t    State;
    byte_t          *Buffer;
};