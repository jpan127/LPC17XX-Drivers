#include <stdio.h>
#include "LPC17xx.h"
#include "scheduler_task.hpp"
#include "task.h"
#include "L5_Application/drivers/gpio_output.hpp"
#include "L5_Application/drivers/buttons.hpp"

#define LED_PORT (GPIO_PORT0)
#define LED_PIN  (1)

// Controls an led, on/off, by 2 button switches
class Switch2LedTask : public scheduler_task
{
public:

	Switch2LedTask(uint8_t priority) : 
										scheduler_task("Switch2LedTask", 2048, priority),
										B0(), 
										B1(), 
										Led(LED_PORT, LED_PIN)
	{
		printf("[Switch2LedTask] Pins are set up.\n");
		Led.SetLow();
	}

	bool run(void *p)
	{	
		// Turn on LED when SW0 is pressed.
		if (B0.IsPressed()) {
			Led.SetHigh();
			printf("[Switch2LedTask] SW0, turning on LED.\n");
		}

		// Turn off LED when SW1 is pressed.
		else if (B1.IsPressed()) {
			Led.SetLow();
			printf("[Switch2LedTask] SW1, turning off LED.\n");
		}

		// Free CPU for context switching
		vTaskDelay(100);

		return true;
	}

private:

	Button0 	B0;
	Button1 	B1;
	GpioOutput 	Led;
};