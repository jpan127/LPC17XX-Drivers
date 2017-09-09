#include "scheduler_task.hpp"
#include "task.h"
#include "L5_Application/drivers/spi.hpp"
using namespace std;

#define SPI_MESSAGE "SPI IS THE BEST PROTOCOL!"

class SpiSendTask : public Spi, public scheduler_task
{
public:

	SpiSendTask(uint8_t priority) : Spi(), scheduler_task("SpiSendTask", 2048, priority)
	{
		Message = SPI_MESSAGE;
	}

	void send_msg(string s)
	{
		puts("Sending message...");

		int length = s.length();
		for (int i=0; i<length; i++) {
			SendByte(s[i]);
			// Small delay otherwise it gets messed up
			vTaskDelay(10);
		}

		puts("Sent message.");
	}

	bool run(void *p)
	{
		// Send the message every 5 seconds
		send_msg(Message);
		vTaskDelay(5000);

		return true;
	}

private:

	string Message;
};

///////////////////////////////////////////////////////////////////////////////////////////////////