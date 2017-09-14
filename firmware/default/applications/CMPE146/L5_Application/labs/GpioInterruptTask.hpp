#pragma once
#include <stdio.h>
#include <scheduler_task.hpp>
#include <task.h>
#include <semphr.h>
#include "L5_Application/drivers/buttons.hpp"
#include "L5_Application/drivers/led.hpp"

typedef void (*void_funct_ptr) (void);	// void function pointer

// Global variables
extern SemaphoreHandle_t Sem;

extern "C"
{
	void EINT2_IRQHandler();
	void EINT3_IRQHandler();
}

class GpioInterruptTask : public scheduler_task
{
public:

	GpioInterruptTask(uint8_t priority);

	bool run(void *p);

	void pins_init();
	void EINT2_init(int pin);
	void EINT3_init(int pin);

};

class sw0_callback : public scheduler_task
{
public:

	sw0_callback() : scheduler_task("sw0_callback", 2048, PRIORITY_LOW)
	{
		puts("sw0_callback.");
	}

	bool run(void *p);
};

class sw1_callback : public scheduler_task
{
public:

	sw1_callback() : scheduler_task("sw1_callback", 2048, PRIORITY_LOW)
	{
		puts("sw1_callback.");
	}

	bool run(void *p);
};