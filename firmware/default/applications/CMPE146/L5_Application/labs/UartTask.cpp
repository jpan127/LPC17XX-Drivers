#include "L5_Application/labs/UartTask.hpp"

// Transmitting message
const char *UART_MESSAGE = "UART IS THE BEST PROTOCOL!";

// Uart error monitor task
void UartErrorTask(void *UartPtr)
{
	while (1)
	{
		if ( ((LPC_UART_TypeDef *)UartPtr)->LSR & LSR_OE_BIT )  {
			printf("[UartErrorTask] Overrun!\n");
		}

		if ( ((LPC_UART_TypeDef *)UartPtr)->LSR & LSR_PE_BIT )  {
			printf("[UartErrorTask] Parity Error!\n");
		}

		if ( ((LPC_UART_TypeDef *)UartPtr)->LSR & LSR_FE_BIT)   {
			printf("[UartErrorTask] Framing Error!\n");
		}

		if ( ((LPC_UART_TypeDef *)UartPtr)->LSR & LSR_BI_BIT)   {
			printf("[UartErrorTask] Break interrupt!\n");
		}

		if ( ((LPC_UART_TypeDef *)UartPtr)->LSR & LSR_RXFE_BIT) {
			printf("[UartErrorTask] RBR Error!\n");
		}

		vTaskDelay(10);
	}
}

UartTask::UartTask(uint8_t priority, uart_port_t port) : 
								scheduler_task("UartTask", 2048, priority),
								Uart(port),
								B0(BUTTON0),
								B1(BUTTON1),
								B2(BUTTON2)
{
	// Initialize with default baud rate
	Init();
	State = TRANSMITTING;
	Buffer = new byte_t[BUFFER_SIZE];

	// Create a monitoring task, passing handle to uart register
	xTaskCreate(&UartErrorTask, "UartErrorTask", 2048, (void *)&UartPtr, 5, NULL);
}

UartTask::~UartTask()
{
	delete [] Buffer;
}

bool UartTask::run(void *p)
{
	// Check for state change
	if ( B0.IsPressed() ) {
		State = TRANSMITTING;
	}
	else if ( B1.IsPressed() ) {
		State = RECEIVING;
	}

	// Transmit or receive
	switch ( State )
	{
		case TRANSMITTING:
			if ( B2.IsPressed() ) {
				SendString(UART_MESSAGE, strlen(UART_MESSAGE));					
			}
			break;

		case RECEIVING:
			if ( RxAvailable() ) {
				size_t rx_size = 0;
				rx_size = ReceiveString(Buffer, BUFFER_SIZE);
				
				// Make sure null terminated string
				if (rx_size < BUFFER_SIZE) {
					Buffer[rx_size] = '\0';
				}
				else {
					Buffer[BUFFER_SIZE-1] = '\0';
				}

				printf("Received: %s\n", Buffer);
			}
			break;
	}

	vTaskDelay(100);

	return true;
}