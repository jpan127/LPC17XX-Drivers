#include "L5_Application/labs/UartTask.hpp"
#include <event_groups.h>

// Global variables
SemaphoreHandle_t UartSem;

// Transmitting message
const char *UART_MESSAGE  = "UART IS THE BEST PROTOCOL!\n";
const char *UART_MESSAGE2 = "UART IS THE WORST PROTOCOL!\n";

// Uart error monitor task
void UartErrorTask(void *UartPtr)
{
	// Create an event group for monitoring
	// Don't want the error to be repeated continuously, clogging up terminal
	// For clarity, the same bits in LSR will set the same bits in this event group
	EventGroupHandle_t UartError_EventGroup = xEventGroupCreate();

	// If bit is set in LSR, and if event group bit is NOT already set,
	// print a message and set the bit
	// Otherwise if bit is set in LSR, and event group bit IS already set,
	// do nothing

	EventBits_t uxBits;
	// Don't wait
	const TickType_t xTicksToWait = 0 / portTICK_PERIOD_MS;
	// All the bits we are testing so we can loop through them
	uint32_t error_bits[5] = {LSR_OE_BIT, LSR_PE_BIT, LSR_FE_BIT, LSR_BI_BIT, LSR_RXFE_BIT};

	while (1)
	{
		// For each error bit
		for (int i=0; i<5; i++)
		{
			// If error bit is set
			if ( ((LPC_UART_TypeDef *)UartPtr)->LSR & error_bits[i] )  
			{
				// Check if bit is set in event group
				uxBits = xEventGroupWaitBits(UartError_EventGroup, 
											(EventBits_t)error_bits[i], 
											pdFALSE, 
											pdFALSE, 
											xTicksToWait);

				// If bit is NOT set in event group, set the bit
				if ( (uxBits & error_bits[i]) != error_bits[i] )
				{
					xEventGroupSetBits(UartError_EventGroup, (EventBits_t)error_bits[i]);
					
					switch (i)
					{
						case 0:	printf("[UartErrorTask] Overrun!\n");			break;
						case 1: printf("[UartErrorTask] Parity Error!\n");		break;
						case 2: printf("[UartErrorTask] Framing Error!\n");		break;
						case 3: printf("[UartErrorTask] Break interrupt!\n");	break;
						case 4: printf("[UartErrorTask] RBR Error!\n");			break;
						default:												break;
					}
				}
			}
			// If no error
			else 
			{
				// Clear bit in event group
				xEventGroupClearBits(UartError_EventGroup, (EventBits_t)error_bits[i]);
			}
		}

		vTaskDelay(10);
	}
}

UartTask::UartTask(uint8_t priority, uart_port_t port) : 
								scheduler_task("UartTask", 8196, priority),
								Uart(port)
{
	// Initialize with default baud rate
	Init();
	// Default state is transmitting
	State = TRANSMITTING;
	// Allocate buffer for RX
	Buffer = new byte_t[BUFFER_SIZE];
	// Create semaphore
	UartSem = xSemaphoreCreateBinary();

	// Create a monitoring task, passing handle to uart register
	// xTaskCreate(&UartErrorTask, "UartErrorTask", 2048, (void *)&UartPtr, PRIORITY_LOW, NULL);
}

UartTask::~UartTask()
{
	delete [] Buffer;
}

bool UartTask::run(void *p)
{
	static bool happy = true;

	// Check for state change
	if ( Button0::getInstance().IsPressed() ) {
		printf("State Change: TRANSMITTING\n");
		State = TRANSMITTING;
	}
	else if ( Button1::getInstance().IsPressed() ) {
		printf("State Change: RECEIVING\n");
		State = RECEIVING;
	}

	// Transmit or receive
	switch ( State )
	{
		case TRANSMITTING:
			if ( Button2::getInstance().IsPressed() ) {
				if (happy) {
					SendString(UART_MESSAGE, strlen(UART_MESSAGE));
					printf("Transmitted: %s", UART_MESSAGE);
				}
				else {
					SendString(UART_MESSAGE2, strlen(UART_MESSAGE2));
					printf("Transmitted: %s", UART_MESSAGE2);
				}
				happy = !happy;
			}
			break;

		case RECEIVING:

			// If interrupt not enabled then simply read RBR
			if ( !(UartPtr->IER & (1 << 0)) ) {
				while ( RxAvailable() ) {
					printf("%c", ReceiveByte());
				}
			}

			// If interrupt enabled then take semaphore + block
			printf("[UartTask] Taking UartSem...\n");
			if ( xSemaphoreTake( UartSem, portMAX_DELAY ) == pdTRUE) {
				for (int i=0; i<RxRingBufferIndex; i++) {
					printf("%c", RxRingBuffer[i]);
				}
				printf("\n");
				// Reset ringbuffer
				RxRingBufferIndex = 0;
			}
		
			break;
	}

	return true;
}