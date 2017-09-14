#pragma once
#include "L5_Application/drivers/gpio.hpp"

// Use for output pins
class GpioOutput : public Gpio
{
public:

	GpioOutput(gpio_port_t port, gpio_pin_t pin) : Gpio(port, pin, OUTPUT)
	{
		/* EMPTY */
	}

    // Sets value high or low
    void SetValue(gpio_value_t value);

    // Set high
    void SetHigh();

    // Set low
    void SetLow();

};