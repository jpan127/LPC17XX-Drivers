#pragma once
#include <stdio.h>
#include <cstdlib>
#include <FreeRTOS.h>
#include <semphr.h>
#include "L5_Application/drivers/gpio_output.hpp"
#include "L5_Application/drivers/led.hpp"

typedef enum { EINT0, EINT1, EINT2, EINT3 } external_interrupt_t;
typedef enum { RISING, FALLING }            interrupt_edge_t;
typedef void (*void_function_ptr_t)();

// Configuration for a GPIO interrupt
typedef struct
{
    external_interrupt_t    eint;
    interrupt_edge_t        edge;
    gpio_port_t             port;
    gpio_pin_t              pin;
    void_function_ptr_t     callback;
} gpio_interrupt_t;

// Global variables
extern SemaphoreHandle_t Sem1;
extern SemaphoreHandle_t Sem2;

// Interrupt Handlers
extern "C"
{
    void EINT2_IRQHandler();
    void EINT3_IRQHandler();
}

class GpioInterrupt : public GpioOutput
{
public:

    // Constructor
    GpioInterrupt(external_interrupt_t  eint, 
                    interrupt_edge_t    edge,
                    gpio_port_t         port, 
                    gpio_pin_t          pin,
                    void_function_ptr_t callback);

    // Constructor with struct
    GpioInterrupt(gpio_interrupt_t gpio_interrupt);

    void RegisterCallback(void_function_ptr_t callback);

private:

    // Initialize interrupt
    void InitializeInterrupt();

    // Struct containing interrupt configuration
    gpio_interrupt_t GpioIntr;
};
