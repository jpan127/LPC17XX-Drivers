#include "led.hpp"

Led::Led(led_t led)
{
	// Save pin number
	switch (led)
	{
		case LED0: Pin = 0; break;
		case LED1: Pin = 1; break;
		case LED2: Pin = 4; break;
		case LED3: Pin = 8; break;
	}

	SetDirectionAsOutput();
	ClearLed();
}

void Led::SetLed()
{
	LPC_GPIO1->FIOSET |= (1 << Pin);
}

void Led::ClearLed()
{
	LPC_GPIO1->FIOCLR |= (1 << Pin);
}

void Led::Blink(int itr)
{
	for (int i=0; i<itr; i++) {
		ClearLed();
		vTaskDelay(50);
		SetLed();
	}
}

void Led::SetDirectionAsOutput()
{
	LPC_GPIO1->FIODIR |= (1 << Pin);
}