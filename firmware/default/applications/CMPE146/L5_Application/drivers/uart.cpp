#include "L5_Application/drivers/uart.hpp"

// Global variables
byte_t  RxByte;
uint8_t RxRingBufferIndex = 0;
byte_t  RxRingBuffer[128] = { 0 };

// Interrupt Handlers
extern "C" 
{
    void UART2_IRQHandler()
    {
        u0_dbg_printf("[UART2_IRQHandler] Interrupt: %i\n", LPC_UART2->IIR & 0x0000000F);
        u0_dbg_printf("[UART2_IRQHandler] Giving UartSem from ISR...\n");

        NVIC_ClearPendingIRQ(UART2_IRQn);
        // Give immediately
        long yield = 0;
        xSemaphoreGiveFromISR(UartSem, &yield);
        // Context switch
        // portYIELD_FROM_ISR((long)0);
    }

    void UART3_IRQHandler()
    {
        // u0_dbg_printf("[UART3_IRQHandler] Giving UartSem from ISR...\n");

        // NVIC_ClearPendingIRQ(UART3_IRQn);

        // RxByte = LPC_UART3->RBR;
        // u0_dbg_printf("Status: %i", LPC_UART3->LSR & (1 << 0));
        // u0_dbg_printf("Byte %c\n", LPC_UART3->RBR);
        // u0_dbg_printf("Status: %i", LPC_UART3->LSR & (1 << 0));

        if (LPC_UART3->LSR & (1 << 0)) {
            u0_dbg_printf("[UART3_IRQHandler] Interrupt: %i\n", LPC_UART3->IIR & 0x0000000F);
            // RxByte = LPC_UART3->RBR;
            // u0_dbg_printf("%c", LPC_UART3->RBR);
            while (LPC_UART3->LSR & (1 << 0)) {
                RxRingBuffer[RxRingBufferIndex] = LPC_UART3->RBR;
                RxRingBufferIndex = (RxRingBufferIndex + 1) % 128;            
            }
                // if (RxRingBufferIndex == 16) {
            //     LPC_UART3->FCR |= (1 << 1);
            // }
            // Give immediately
            long yield = 0;
            xSemaphoreGiveFromISR(UartSem, &yield);
            portYIELD_FROM_ISR(yield);
        }

        // u0_dbg_printf("[UART3_IRQHandler] Yield: %i\n", (int)yield);

        // if (yield != pdFALSE)
        // {
            // Context switch
        // }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Uart::Uart(uart_port_t port)
{
    Port = port;

    switch (Port)
    {
        // case UART_PORT0: 
        //     UartPtr = (LPC_UART_TypeDef *)LPC_UART0; 
        //     IRQPtr  = UART0_IRQn;
        //     break;
        case UART_PORT2: 
            UartPtr = LPC_UART2; 
            IRQPtr  = UART2_IRQn;
            break;
        case UART_PORT3: 
            UartPtr = LPC_UART3; 
            IRQPtr  = UART3_IRQn;
            break;
    }
}

void Uart::Init(uint32_t baud_rate)
{
    switch (Port)
    {
        // case UART_PORT0:
        //     LPC_SC->PCONP       |=  (1 << 3);
        //     LPC_SC->PCLKSEL0    &= ~(3 << 6);
        //     LPC_SC->PCLKSEL0    |=  (1 << 6);
        //     LPC_PINCON->PINSEL0 &= ~(0xF << 4);
        //     LPC_PINCON->PINSEL0 |=  (0x5 << 4);
        //     break;

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
    // Enable RX available interrupt
    UartPtr->IER = (1 << 0);
    // Enable DLAB before configuration
    UartPtr->LCR = (1 << 7);
    // Clear FIFOs
    UartPtr->FCR = 0x7;
    UartPtr->DLM = 0;
    UartPtr->DLL = sys_get_cpu_clock() / (16 * baud_rate);
    // 8-bit character, 1 stop bit, no parity, no break, disable DLAB
    UartPtr->LCR = 3;
    
    NVIC_EnableIRQ(IRQPtr);

    int p = -1;

    switch (Port)
    {
        case UART_PORT2: p = 2;  break;
        case UART_PORT3: p = 3;  break;
        default:         p = -1; break;
    }

    printf("Uart %i initialized.\n", p);
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
        vTaskDelay(10);
    }

    return (size_t)index;
}

byte_t Uart::ReceiveByte()
{
    return UartPtr->RBR;
}