#pragma once
#include <cassert>
#include <stdio.h>
#include <LPC17xx.h>
#include <FreeRTOS.h>
#include <task.h>
#include <singleton_template.hpp>

typedef enum { LED0, LED1, LED2, LED3 } led_t;

// Helper functions for changing all LEDs at once
void LedsToggleAll();
void LedsSetAll();
void LedsClearAll();

class Led
{
public:

	// Turn on
	void SetLed();

	// Turn off
	void ClearLed();

	// Blink
	void Blink(int itr);

	// Toggle
	void Toggle();

protected:

	// Constructor
	Led(led_t led);

private:

	// Set direction as output only once
	void SetDirectionAsOutput();

	uint8_t Pin;
};

class Led0 : public Led, public SingletonTemplate <Led0>
{
private:

	Led0() : Led(LED0) 
	{
		// printf("Initialized LED0.\n");
	}

	friend class SingletonTemplate <Led0>;
};

class Led1 : public Led, public SingletonTemplate <Led1>
{
private:

	Led1() : Led(LED1) 
	{
		// printf("Initialized LED1.\n");
	}

	friend class SingletonTemplate <Led1>;
};

class Led2 : public Led, public SingletonTemplate <Led2>
{
private:

	Led2() : Led(LED2) 
	{
		// printf("Initialized LED2.\n");
	}

	friend class SingletonTemplate <Led2>;
};

class Led3 : public Led, public SingletonTemplate <Led3>
{
private:

	Led3() : Led(LED3) 
	{
		// printf("Initialized LED3.\n");
	}

	friend class SingletonTemplate <Led3>;
};