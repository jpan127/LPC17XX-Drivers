#pragma once
#include <stdio.h>
#include <scheduler_task.hpp>
#include <task.h>
#include <semphr.h>
#include "L5_Application/drivers/buttons.hpp"
#include "L5_Application/drivers/led.hpp"
#include "L5_Application/drivers/gpio_interrupt.hpp"

// [TASK] Waits for Callback1 to unblock from ISR
void InterruptCallbackTask1(void *p)
{
	vTaskDelay(10);

	while (1)
	{
		printf("[InterruptCallbackTask1] Semaphore Taken!\n");
		if ( xSemaphoreTake(EINT2_Sem, portMAX_DELAY) ) {
			printf("[InterruptCallbackTask1] Semaphore Given!\n");
		}
	}
}

// [TASK] Waits for Callback2 to unblock from ISR
void InterruptCallbackTask2(void *p)
{
	vTaskDelay(20);
	
	while (1)
	{
		printf("[InterruptCallbackTask2] Semaphore Taken!\n");
		if ( xSemaphoreTake(EINT3_Sem, portMAX_DELAY) ) {
			printf("[InterruptCallbackTask2] Semaphore Given!\n");
		}
	}
}

// [ISR] Unblocks Callback1
void Callback1()
{
	long yield;
	xSemaphoreGiveFromISR(EINT2_Sem, &yield);
	portYIELD_FROM_ISR(yield);
}

// [ISR] Unblocks Callback2
void Callback2()
{
	long yield;
	xSemaphoreGiveFromISR(EINT3_Sem, &yield);
	portYIELD_FROM_ISR(yield);
}

// Triggers ISR that triggers callback
class GpioInterruptTask : public scheduler_task, public GpioInterrupt
{
public:

	GpioInterruptTask(uint8_t priority, gpio_interrupt_t gpio_interrupt) :
											scheduler_task("Interrupt_Lab", 2048, priority),
											GpioInterrupt(gpio_interrupt)
	{
		/* EMPTY */
	}

	bool run(void *p)
	{
		if ( Button0::getInstance().IsPressed() ) {
			puts("SW0 pressed");
			// Set interrupt pin high to trigger
			SetHigh();
		}

		if ( Button1::getInstance().IsPressed() ) {
			Led0::getInstance().ClearLed();
			Led1::getInstance().ClearLed();
			Led2::getInstance().ClearLed();
			Led3::getInstance().ClearLed();
		}

		if ( Button2::getInstance().IsPressed() ) {
			Led0::getInstance().SetLed();
			Led1::getInstance().SetLed();
			Led2::getInstance().SetLed();
			Led3::getInstance().SetLed();
		}

		vTaskDelay(100);
		return true;
	}
};