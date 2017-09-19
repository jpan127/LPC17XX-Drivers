#include <stdio.h>
#include "LPC17xx.h"
#include "scheduler_task.hpp"
#include "task.h"
#include "L5_Application/drivers/gpio_output.hpp"
#include "L5_Application/drivers/buttons.hpp"
#include "L5_Application/drivers/led.hpp"

#define LED_PORT 		(GPIO_PORT0)
#define SWITCH_PORT 	(GPIO_PORT0)
#define LED_PIN 		(1)
#define SWITCH_IN_PIN 	(0)

// Controls an led, on/off, by 2 button switches
class Switch2LedTask : public scheduler_task
{
public:

	Switch2LedTask(uint8_t priority) : 
										scheduler_task("Switch2LedTask", 2048, priority),
										Led(LED_PORT, LED_PIN),
										ExternalSwitchIn(SWITCH_PORT, SWITCH_IN_PIN)
	{
		Led.SetLow();
		printf("[Switch2LedTask] Pins are set up.\n");
	}

	bool run(void *p)
	{ 
		// Turn on LED when SW0 is pressed.
		if ( Button0::getInstance().IsPressed() ) {
		Led0::getInstance().Toggle();
		Led1::getInstance().Toggle();
		Led2::getInstance().Toggle();
		Led3::getInstance().Toggle();
		printf("[Switch2LedTask] On-board switch, toggling LED.\n");
	}

	// Turn off LED when SW1 is pressed.
	else if ( ExternalSwitchIn.IsHigh() ) {
		ExternalSwitchIn.Debounce();
		Led.Toggle();
		printf("[Switch2LedTask] External switch, toggling LED.\n");
	}

	vTaskDelay(100);
	return true;
	}

private:

	// Gpio P0.1
	GpioOutput Led;
	// Gpio P0.0
	GpioInput ExternalSwitchIn;
};