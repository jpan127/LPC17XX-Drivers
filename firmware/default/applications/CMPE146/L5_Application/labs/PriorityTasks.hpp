#pragma once
#include <uart0_min.h>
#include "L5_Application/drivers/buttons.hpp"

/*///////////////////////////////////////////////////////////////////////////////////////////////////////
1. The RTOS generates an interrupt every clock tick, and this causes same priority tasks to context switch
round robin.
2. The tasks only print out 3-4 characters because that is as much as it can print out
before the tick interrupt (1ms) is up.
3. The reason why sometimes a task prints 3 characters instead of 4 is because the speed/frequency of 
the uart is not exact.  It cycles in and out of phase periodically and when it either synchronizes
in phase or synchronizes out of phase it produces a slightly different behavior.
4. When the tasks call vTaskDelay() they yield their cpu time and allow for context switching.  Since the
tasks are not printing in parallel, when one task is the first to yield, the other task is most likely
not finished and will print out the remainder of its message.
5. When one task is higher priority than the other, a tick interrupt does not trigger a context switch,
and therefore the higher priority task will finish.  When it finishes it will call vTaskDelay() and yield,
which will allow the lower priority task to start printing.  Without this vTaskDelay() the higher priority
task can starve the lower priority of CPU time.
*///////////////////////////////////////////////////////////////////////////////////////////////////////

const char o[] = 	"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"	// 32 characters
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
					"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO";	// 160 characters total
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