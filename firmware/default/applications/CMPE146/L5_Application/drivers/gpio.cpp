#include "gpio.hpp"

Gpio::Gpio(gpio_port_t port, gpio_pin_t pin, gpio_mode_t mode)
{
	// Loose check to see if pin number is out of range
	assert(pin <= 31);

	// Save port and pin internally
	Port = port;
	Pin  = pin;

	// Get pointer to gpio port
	LPC_GPIO_TypeDef *gpio_ptr = GetPointer();
	assert(gpio_ptr != NULL);

	// Set direction of pin
	switch (mode)
	{
		case INPUT:  gpio_ptr->FIODIR &= ~(1 << Pin); break;
		case OUTPUT: gpio_ptr->FIODIR |=  (1 << Pin); break;
	}

	delete gpio_ptr;
}

void Gpio::SetValue(gpio_value_t value)
{
	// Get pointer to gpio port
	LPC_GPIO_TypeDef *gpio_ptr = GetPointer();
	assert(gpio_ptr != NULL);

	switch (value)
	{
		case HIGH: gpio_ptr->FIOSET |= (1 << Pin); break;
		case LOW:  gpio_ptr->FIOCLR |= (1 << Pin); break;
	}

	delete gpio_ptr;
}

void Gpio::SetHigh()
{
	LPC_GPIO_TypeDef *gpio_ptr = GetPointer();
	assert(gpio_ptr != NULL);

	gpio_ptr->FIOSET |= (1 << Pin);

	delete gpio_ptr;
}

void Gpio::SetLow()
{
	LPC_GPIO_TypeDef *gpio_ptr = GetPointer();
	assert(gpio_ptr != NULL);

	gpio_ptr->FIOCLR |= (1 << Pin);

	delete gpio_ptr;
}

bool Gpio::IsHigh()
{
	LPC_GPIO_Typedef *gpio_ptr = GetPointer();

	bool high = (gpio_ptr->FIOPIN & (1 << Pin));

	delete gpio_ptr;
	return high;
}

bool Gpio::IsLow()
{
	LPC_GPIO_Typedef *gpio_ptr = GetPointer();

	bool low = !(gpio_ptr->FIOPIN & (1 < Pin));
	
	delete gpio_ptr;
	return low;
}

LPC_GPIO_Typedef* Gpio::GetPointer()
{

	switch (Port)
	{
		case GPIO_PORT0: return LPC_GPIO0; break;
		case GPIO_PORT1: return LPC_GPIO1; break;
		case GPIO_PORT2: return LPC_GPIO2; break;
		case GPIO_PORT3: return LPC_GPIO3; break;

		default:
			assert(0 && "Unhandled GPIO port enum case!");
			return NULL;
	}
}