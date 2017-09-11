#include "gpio.hpp"

Gpio::Gpio(gpio_port_t port, gpio_pin_t pin, gpio_mode_t mode)
{
	// Loose check to see if pin number is out of range
	if (pin > 31) {
		printf("Invalid Pin: %i\n", pin);
		assert(pin <= 31);
	}

	// Save port and pin internally
	Port = port;
	Pin  = pin;

	// Select GPIO functionality for specified pin
	switch (Port)
	{
		case GPIO_PORT0:
			if (pin <= 15) 
			{
				LPC_PINCON->PINSEL0 &= ~(0x3 << (2*pin));
			}
			else if (pin <= 31) 
			{
				LPC_PINCON->PINSEL1 &= ~(0x3 << (2*(pin - 16)));
			}
			break;

		case GPIO_PORT1:
			if (pin <= 15) 
			{
				// Can't initialize reserved pins
				bool reserved = false;
				switch (Pin)
				{
					case 2:	 reserved = true; break;
					case 3:	 reserved = true; break;
					case 5:	 reserved = true; break;
					case 6:	 reserved = true; break;
					case 7:	 reserved = true; break;
					case 11: reserved = true; break;
					case 12: reserved = true; break;
					case 13: reserved = true; break;
					default: reserved = false;
				}

				if (reserved) 
				{
					printf("Port: %i Pin: %i is reserved, cannot be used as GPIO.\n", port, pin);
					return;
				}
				else 
				{
					LPC_PINCON->PINSEL2 &= ~(0x3 << (2*pin));
				}
			}
			else if (pin <= 31)
			{
				LPC_PINCON->PINSEL3 &= ~(0x3 << (2*(pin - 16)));
			}
			break;

		case GPIO_PORT2:
			if (pin <= 13)
			{
				LPC_PINCON->PINSEL4 &= ~(0x3 << (2*pin));
			}
			break;

		default:
			// Not handling
			break;
	}


	switch (Port)
	{
		case GPIO_PORT0: GpioPtr = LPC_GPIO0; break;
		case GPIO_PORT1: GpioPtr = LPC_GPIO1; break;
		case GPIO_PORT2: GpioPtr = LPC_GPIO2; break;
		case GPIO_PORT3: GpioPtr = LPC_GPIO3; break;
	}

	// Set direction of pin
	switch (mode)
	{
		case INPUT:  GpioPtr->FIODIR &= ~(1 << Pin); break;
		case OUTPUT: GpioPtr->FIODIR |=  (1 << Pin); break;
	}
}

void Gpio::SetValue(gpio_value_t value)
{
	switch (value)
	{
		case HIGH: GpioPtr->FIOSET |= (1 << Pin); break;
		case LOW:  GpioPtr->FIOCLR |= (1 << Pin); break;
	}
}

void Gpio::SetHigh()
{
	SetValue(HIGH);
}

void Gpio::SetLow()
{
	SetValue(LOW);
}

bool Gpio::IsHigh()
{
	return (GpioPtr->FIOPIN & (1 << Pin));
}

bool Gpio::IsLow()
{
	return !(GpioPtr->FIOPIN & (1 < Pin));
}