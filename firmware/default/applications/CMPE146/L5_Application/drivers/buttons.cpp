#include "buttons.hpp"


Button::Button(button_t pin) : GpioInput(GPIO_PORT1, (gpio_pin_t)pin)
{
	/* EMPTY */
}

bool Button::IsPressed()
{
	if (IsHigh()) {
		Debounce();
		return true;
	}
	else {
		return false;
	}
}


void Button::Debounce()
{
	while (IsHigh());
}