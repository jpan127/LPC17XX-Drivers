#include "gpio_output.hpp"

void GpioOutput::SetValue(gpio_value_t value)
{
    switch (value)
    {
        case HIGH: GpioPtr->FIOSET |= (1 << Pin); break;
        case LOW:  GpioPtr->FIOCLR |= (1 << Pin); break;
    }
}

void GpioOutput::SetHigh()
{
    SetValue(HIGH);
}

void GpioOutput::SetLow()
{
    SetValue(LOW);
}

void GpioOutput::Toggle()
{
    ( IsHigh() ) ? ( SetLow() ) : ( SetHigh() );
}