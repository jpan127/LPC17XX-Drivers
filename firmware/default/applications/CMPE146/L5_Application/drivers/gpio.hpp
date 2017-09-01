#pragma once
#include <cassert>
#include "LPC17xx.h"

#ifndef NULL
#define NULL (0)
#endif

typedef uint8_t 							gpio_pin_t;
typedef enum {PORT0, PORT1, PORT2, PORT3} 	gpio_port_t;
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
	virtual void SetValue(gpio_value_t value);

	// Set high
	virtual void SetHigh();

	// Set low
	virtual void SetLow();

	// Check if high
	virtual bool IsHigh();

	// Check if low
	virtual bool IsLow();

	// Blocks until pin goes low
	virtual void Debounce();

private:

	// Get pointer to gpio register
	LPC_GPIO_Typedef* GetPointer();

	gpio_pin_t 	Pin;
	gpio_port_t 	Port;
};