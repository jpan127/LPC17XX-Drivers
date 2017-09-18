#pragma once
#include <uart0_min.h>
#include "L5_Application/drivers/buttons.hpp"

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////
	The RTOS genereates an interrupt every clock tick, and this causes same priority tasks to context switch.
	The reason why sometimes a task prints 3 characters instead of 4 is because the speed/frequency of 
	the uart is not exact.  It cycles in and out of phase periodically and when it either synchronizes
	in phase or synchronizes out of phase it produces a slightly different behavior.
	When the tasks call vTaskDelay() they yield their cpu time and allow for context switching.  Since the
	tasks are not printing in sync or equally, when one task first yields, the other task is most likely
	not finished and will print out the remainder of its message.
*////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char o[] = 	"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"	// 32 characters
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO";
const char x[] = 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
					"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
					"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
					"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
					"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

// [TASK]
void PriorityTaskLow(void *p)
{
	TickType_t delay = 50;

	while (1)
	{
		if ( Button0::getInstance().IsPressed() ) {
			delay = 1000;
		}
		else if ( Button1::getInstance().IsPressed() ) {
			delay = 50;
		}
		uart0_puts(o);
		vTaskDelay(delay);
	}
}

// [TASK]
void PriorityTaskHigh(void *p)
{
	TickType_t delay = 50;

	while (1)
	{
		if ( Button0::getInstance().IsPressed() ) {
			delay = 1000;
		}
		else if ( Button1::getInstance().IsPressed() ) {
			delay = 50;
		}
		uart0_puts(x);
		vTaskDelay(delay);
	}
}