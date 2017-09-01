#include <stdio.h>
#include "LPC17xx.h"
#include "scheduler_task.hpp"
#include "task.h"
#include "gpio_input.hpp"
#include "gpio_output.hpp"
using namespace std;


class Switch2LedTask : public scheduler_task
{
public:

	Switch2LedTask(uint8_t priority);

	bool run(void *p);

private:

	GpioInput 	Sw0;
	GpioInput 	Sw1;
	GpioOutput 	Led;
};

/////////////////////////////////////////////////////////////////////////////////

Switch2LedTask::Switch2LedTask(uint8_t priority) : 
								scheduler_task("Switch2LedTask", 2048, priority),
								Sw0(PORT1, 9), Sw1(PORT1, 10), Led(PORT0, 1)
{
	printf("Pins are set up.\n");
}

bool Switch2LedTask::run(void *p)
{
	if (Sw0.IsHigh()) {
		Sw0.Debounce();
		Led.SetHigh();
		printf("SW0, turning on LED.\n");
	}

	else if (Sw1.IsHigh()) {
		Sw1.Debounce();
		Led.SetLow();
		printf("SW1, turning off LED.\n");
	}

	return true;
}