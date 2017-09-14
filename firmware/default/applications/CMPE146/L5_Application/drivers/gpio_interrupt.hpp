#pragma once
#include "L5_Application/drivers/gpio_input.hpp"

class GpioInterrupt : public GpioInput
{
public:

    // Constructor
    GpioInterrupt(gpio_port_t port, gpio_pin_t pin);

private:

    // Initialize interrupt
    void InitializeInterrupt(gpio_port_t port, gpio_pin_t pin);
};

GpioInterrupt::GpioInterrupt(gpio_port_t port, gpio_pin_t pin) : GpioInput(port, pin)
{
    InitializeInterrupt(port, pin);
}

void InitializeInterrupt(gpio_port_t port, gpio_pin_t pin)
{
    NVIC_DisableIRQ(EINT3_IRQn);

    LPC_SC->EXTMODE  = (1 << pin);
    LPC_SC->EXTPOLAR = (1 << pin);
    LPC_SC->EXTINT   = (1 << pin);

    switch (port)
    {
        case GPIO_PORT0:
            LPC_GPIOINT->IO0IntEnR  = (1 << pin);
            LPC_GPIOINT->IO0IntEnF  = 0;
            break;

        case GPIO_PORT2:
            LPC_GPIOINT->IO2IntEnR  = (1 << pin);
            LPC_GPIOINT->IO2IntEnF  = 0;
            break;        
    }

    NVIC_EnableIRQ(EINT3_IRQn);

}