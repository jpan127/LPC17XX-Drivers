#include <stdio.h>
#include <scheduler_task.hpp>
#include <task.h>
#include "L5_Application/drivers/spi.hpp"
#include "L5_Application/drivers/buttons.hpp"

#define SPI_MESSAGE ("SPI IS THE BEST PROTOCOL!")
#define SPI_MODE	(SPI_MASTER)

class AT45QueryTask : public scheduler_task, public AT45DB161
{
public:

	AT45QueryTask(uint8_t priority) : scheduler_task("AT45QueryTask", 2048, priority),
										AT45DB161()									
	{
		/* EMPTY */
	}

	bool run(void *p)
	{
		if ( Button0::getInstance().IsPressed() ) {
		}
		else if ( Button1::getInstance().IsPressed() ) {
			// Read status register
			ReadStatusRegister();
		}
		else if ( Button2::getInstance().IsPressed() ) {
			// Read manufacturer ID + device ID
			ReadManufacturerID();
		}
		else if ( Button3::getInstance().IsPressed() ) {
		}

		vTaskDelay(100);
		return true;
	}

private:

};

///////////////////////////////////////////////////////////////////////////////////////////////////