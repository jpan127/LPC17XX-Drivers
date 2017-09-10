#include "L5_Application/drivers/uart.hpp"

Uart::Uart(uart_port_t port)
{
    Port = port;

    switch (Port)
    {
        case UART_PORT0: UartPtr = (LPC_UART_TypeDef *)LPC_UART0; break;
        case UART_PORT2: UartPtr = LPC_UART2; break;
        case UART_PORT3: UartPtr = LPC_UART3; break;
    }
}

void Uart::Init(uint32_t baud_rate)
{
    switch (Port)
    {
        case UART_PORT0:
            LPC_SC->PCONP       |=  (1 << 3);
            LPC_SC->PCLKSEL0    &= ~(3 << 6);
            LPC_SC->PCLKSEL0    |=  (1 << 6);
            LPC_PINCON->PINSEL0 &= ~(0xF << 4);
            LPC_PINCON->PINSEL0 |=  (0x5 << 4);
            break;

        // case UART_PORT1:
        //     LPC_SC->PCONP       |=  (1 << 4);
        //     LPC_SC->PCLKSEL0    &= ~(3 << 8);
        //     LPC_SC->PCLKSEL0    |=  (1 << 8);
        //     LPC_PINCON->PINSEL0 &= ~(3 << 30);  // TXD1 is PINSEL0
        //     LPC_PINCON->PINSEL1 &= ~(3 <<  0);  // RXD1 is PINSEL1
        //     LPC_PINCON->PINSEL0 |=  (1 << 30);  // Set pins as 01 01
        //     LPC_PINCON->PINSEL0 |=  (1 <<  0);
        //     break;

        case UART_PORT2:
            LPC_SC->PCONP       |=  (1 << 24);
            LPC_SC->PCLKSEL1    &= ~(3 << 16);
            LPC_SC->PCLKSEL1    |=  (1 << 16);
            LPC_PINCON->PINSEL0 &= ~(0xF << 20);
            LPC_PINCON->PINSEL0 |=  (0x5 << 20); // Set pins as 01 01
            break;

        case UART_PORT3:
            LPC_SC->PCONP       |=  (1 << 25);
            LPC_SC->PCLKSEL1    &= ~(3 << 18);
            LPC_SC->PCLKSEL1    |=  (1 << 18);
            LPC_PINCON->PINSEL0 &= ~(0xF << 0);
            LPC_PINCON->PINSEL0 |=  (0xA << 0); // Set pins as 10 10
            break;
    }

    // Same configuration across all UART, avoid repetition
    // Enable DLAB before configuration
    UartPtr->LCR = (1 << 7);
    UartPtr->DLM = 0;
    UartPtr->DLL = sys_get_cpu_clock() / (16 * baud_rate);
    // 8-bit character, 1 stop bit, no parity, no break, disable DLAB
    UartPtr->LCR = 3;

    printf("Uart %i initialized.\n", Port);
}

bool Uart::TxAvailable()
{
    return ( UartPtr->LSR & LSR_THRE_BIT );
}

void Uart::SendString(byte_t *buffer, size_t buffer_size)
{
    if (buffer == NULL) {
        printf("[Uart::SendString] Input buffer null.\n");
        return;
    }

    for (size_t i=0; i<buffer_size; i++) {
        SendByte(buffer[i]);
        vTaskDelay(10);
    }
}

void Uart::SendString(const char *buffer, size_t buffer_size)
{
    if (buffer == NULL) {
        printf("[Uart::SendString] Input buffer null.\n");
        return;
    }

    for (size_t i=0; i<buffer_size; i++) {
        SendByte(buffer[i]);
        vTaskDelay(10);
    }
}

void Uart::SendByte(byte_t c)
{
    // Put byte into transmitting register
    UartPtr->THR = c;
    // Wait until it is transmitted completely, THR empty
    while ( !TxAvailable() );
}

bool Uart::RxAvailable()
{
    return ( UartPtr->LSR & LSR_RDR_BIT );
}

size_t Uart::ReceiveString(byte_t *buffer, size_t buffer_size)
{
    if (buffer == NULL) {
        printf("[Uart::ReceiveString] Input buffer null.\n");
        return 0;
    }

    unsigned int index = 0;

    while ( RxAvailable() ) {
        if (index < buffer_size) {
            buffer[index++] = ReceiveByte();
        }
    }

    return (size_t)index;
}

byte_t Uart::ReceiveByte()
{
    return UartPtr->RBR;
}