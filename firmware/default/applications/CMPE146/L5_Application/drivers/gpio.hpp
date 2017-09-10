#pragma once
#include <cassert>
#include <stdio.h>
#include "LPC17xx.h"

#ifndef NULL
#define NULL (0)
#endif

typedef uint8_t 							gpio_pin_t;
typedef enum {GPIO_PORT0, GPIO_PORT1,
			  GPIO_PORT2, GPIO_PORT3} 		gpio_port_t;
typedef enum {INPUT, OUTPUT} 				gpio_mode_t;
typedef enum {HIGH, LOW} 					gpio_value_t;
// Forward declaration necessary for linkage
typedef LPC_GPIO_type LPC_GPIO_Typedef;


// Gpio base class, don't use
class Gpio
{
public:

	// Constructor: port + pin + mode
	Gpio(gpio_port_t port, gpio_pin_t pin, gpio_mode_t mode);

	// Sets value high or low
	void SetValue(gpio_value_t value);

	// Set high
	void SetHigh();

	// Set low
	void SetLow();

	// Check if high
	bool IsHigh();

	// Check if low
	bool IsLow();

private:

	// Get pointer to gpio register
	LPC_GPIO_Typedef* GetPointer();

	LPC_GPIO_Typedef *GpioPtr;
	gpio_pin_t 		Pin;
	gpio_port_t 	Port;
};