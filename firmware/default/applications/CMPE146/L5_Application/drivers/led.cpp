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
	LPC_GPIO1->FIOCLR |= (1 << Pin);
}

void Led::ClearLed()
{
	LPC_GPIO1->FIOSET |= (1 << Pin);
}

void Led::Blink(int itr)
{
	for (int i=0; i<itr; i++) {
		ClearLed();
		vTaskDelay(50);
		SetLed();
	}
}

void Led::Toggle()
{
	// If cleared, set
	if (LPC_GPIO1->FIOPIN & (1 << Pin)) {
		SetLed();
	}
	// Else if set, clear
	else {
		ClearLed();
	}
}

void Led::SetDirectionAsOutput()
{
	LPC_GPIO1->FIODIR |= (1 << Pin);
}

void LedsToggleAll()
{
    Led0::getInstance().Toggle();
    Led1::getInstance().Toggle();
    Led2::getInstance().Toggle();
    Led3::getInstance().Toggle();
}

void LedsSetAll()
{
	Led0::getInstance().SetLed();
	Led1::getInstance().SetLed();
	Led2::getInstance().SetLed();
	Led3::getInstance().SetLed();
}

void LedsClearAll()
{
	Led0::getInstance().ClearLed();
	Led1::getInstance().ClearLed();
	Led2::getInstance().ClearLed();
	Led3::getInstance().ClearLed();
}