#include <stdio.h>
#include <scheduler_task.hpp>
#include <task.h>
#include "L5_Application/drivers/spi.hpp"
#include "L5_Application/drivers/buttons.hpp"

#define SPI_MESSAGE ("SPI IS THE BEST PROTOCOL!")
#define SPI_PORT	(SPI_PORT0)
#define SPI_MODE	(SPI_MASTER)

typedef enum { TRANSMITTING, RECEIVING } spi_state_t;

class SpiSendTask : public Spi, public scheduler_task
{
public:

	SpiSendTask(uint8_t priority) : Spi(SPI_PORT, SPI_MODE), 
									scheduler_task("SpiSendTask", 2048, priority)
	{
		/* EMPTY */
	}

	// void SendMessage(string s)
	// {
	// 	puts("Sending message...");

	// 	int length = s.length();
	// 	for (int i=0; i<length; i++) {
	// 		SendByte(s[i]);
	// 		// Small delay otherwise it gets messed up
	// 		vTaskDelay(10);
	// 	}

	// 	puts("Sent message.");
	// }

	bool run(void *p)
	{
		// Check for state change
		if ( Button0::getInstance().IsPressed() && State != TRANSMITTING ) {
			printf("State Change: TRANSMITTING\n");
			State = TRANSMITTING;
		}
		else if ( Button1::getInstance().IsPressed() && State != RECEIVING ) {
			printf("State Change: RECEIVING\n");
			State = RECEIVING;
		}

		switch ( State )
		{
			case TRANSMITTING:

				if ( Button2::getInstance().IsPressed() ) {
					SendString(SPI_MESSAGE, strlen(SPI_MESSAGE));
				}
				break;

			case RECEIVING:

				while ( !RxAvailable() ) {
					vTaskDelay(10);
				}
				while ( RxAvailable() ) {
					printf("%c", ReceiveByte());
				}
				break;				
		}

		vTaskDelay(100);
		return true;
	}

private:

	spi_state_t State; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////