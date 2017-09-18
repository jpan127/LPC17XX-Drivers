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
	vTaskDelay(100);

	while (1)
	{
		vTaskDelay(100);

		printf("[InterruptCallbackTask1] Semaphore [1] Taken!\n");
		if ( xSemaphoreTake(Sem1, portMAX_DELAY) ) {
			printf("[InterruptCallbackTask1] Semaphore [1] Given!\n");
		}
	}
}

// [TASK] Waits for Callback2 to unblock from ISR
void InterruptCallbackTask2(void *p)
{
	vTaskDelay(200);
	
	while (1)
	{
		vTaskDelay(200);

		printf("[InterruptCallbackTask2] Semaphore [2] Taken!\n");
		if ( xSemaphoreTake(Sem2, portMAX_DELAY) ) {
			printf("[InterruptCallbackTask2] Semaphore [2] Given!\n");
		}
	}
}

// [ISR] Unblocks Callback1
void Callback1()
{
	long yield;
	xSemaphoreGiveFromISR(Sem1, &yield);
	portYIELD_FROM_ISR(yield);
}

// [ISR] Unblocks Callback2
void Callback2()
{
	long yield;
	xSemaphoreGiveFromISR(Sem2, &yield);
	portYIELD_FROM_ISR(yield);
}

// Triggers ISR that triggers callback
class GpioInterruptTask : public scheduler_task
{
public:

	GpioInterruptTask(uint8_t priority, gpio_interrupt_t gpio_interrupt1, 
										gpio_interrupt_t gpio_interrupt2) :
										scheduler_task("Interrupt_Lab", 2048, priority),
										GpioInterrupt1(gpio_interrupt1),
										GpioInterrupt2(gpio_interrupt2)
	{
		/* EMPTY */
	}

	bool run(void *p)
	{
		if ( Button0::getInstance().IsPressed() ) {
			puts("SW0 pressed");
			// Set interrupt pin high to trigger
			GpioInterrupt1.SetHigh();
		}

		if ( Button1::getInstance().IsPressed() ) {
			puts("SW1 pressed");
			// Set interrupt pin high to trigger
			GpioInterrupt2.SetHigh();
		}

		if ( Button2::getInstance().IsPressed() ) {
			Led0::getInstance().ClearLed();
			Led1::getInstance().ClearLed();
			Led2::getInstance().ClearLed();
			Led3::getInstance().ClearLed();
		}

		if ( Button3::getInstance().IsPressed() ) {
			Led0::getInstance().SetLed();
			Led1::getInstance().SetLed();
			Led2::getInstance().SetLed();
			Led3::getInstance().SetLed();
		}

		vTaskDelay(100);
		return true;
	}

private:

	GpioInterrupt GpioInterrupt1;
	GpioInterrupt GpioInterrupt2;
};