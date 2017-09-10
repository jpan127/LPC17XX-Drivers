#pragma once
#include <stdio.h>
#include "L5_Application/drivers/gpio_input.hpp"

typedef enum {BUTTON0, BUTTON1, BUTTON2, BUTTON3} button_t;

#define B0_PIN (9)
#define B1_PIN (10)
#define B2_PIN (14)
#define B3_PIN (15)

class Button : public GpioInput
{
public:

	// Constructor
	Button(button_t button);

	// Returns true if pressed, and if pressed blocks/debounces until unpressed
	bool IsPressed();

private:

	// Waits for pin to go low
	void Debounce();
	
	gpio_pin_t 	Pin;
};