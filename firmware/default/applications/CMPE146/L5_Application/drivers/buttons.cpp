#include "buttons.hpp"


Button::Button(button_t button) : GpioInput(GPIO_PORT1, Pin)
{
	switch (button)
	{
		case BUTTON0: Pin = B0_PIN; printf("Button 0 initialized.\n"); break;
		case BUTTON1: Pin = B1_PIN; printf("Button 1 initialized.\n"); break;
		case BUTTON2: Pin = B2_PIN; printf("Button 2 initialized.\n"); break;
		case BUTTON3: Pin = B3_PIN; printf("Button 3 initialized.\n"); break;
	}
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