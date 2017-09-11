#pragma once
#include <stdio.h>
#include "L5_Application/drivers/gpio_input.hpp"

typedef enum 
{
    BUTTON0 = 9,
    BUTTON1 = 10,
    BUTTON2 = 14,
    BUTTON3 = 15
} button_t;

// Base class
class Button : public GpioInput
{
public:

	// Returns true if pressed, and if pressed blocks/debounces until unpressed
	bool IsPressed();

protected:

    // Constructor
    Button(button_t pin);

private:
    
	// Waits for pin to go low
	void Debounce();
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Button0 : public Button
{
public:

    Button0() : Button(BUTTON0)
    {
        printf("Button 0 initialized.\n");
    }
};

class Button1 : public Button
{
public:

    Button1() : Button(BUTTON1)
    {
        printf("Button 1 initialized.\n");
    }
};

class Button2 : public Button
{
public:

    Button2() : Button(BUTTON2)
    {
        printf("Button 2 initialized.\n");
    }
};

class Button3 : public Button
{
public:

    Button3() : Button(BUTTON3)
    {
        printf("Button 3 initialized.\n");
    }
};