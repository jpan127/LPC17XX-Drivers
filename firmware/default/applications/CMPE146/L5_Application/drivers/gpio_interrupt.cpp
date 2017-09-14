#include "gpio_interrupt.hpp"

// Global variables
SemaphoreHandle_t EINT2_Sem = xSemaphoreCreateBinary();
SemaphoreHandle_t EINT3_Sem = xSemaphoreCreateBinary();
// Initialize pointer to array of pointers, pointing to structs, initialized NULL
static gpio_interrupt_t **gpio_port0_interrupts = new gpio_interrupt_t*[31]();
static gpio_interrupt_t **gpio_port2_interrupts = new gpio_interrupt_t*[14]();

///////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
    void EINT2_IRQHandler()
    {
        // Clear all active interrupts for GPIO port 0
        for (int i=0; i<31; i++) {
            // Reserved bits [14:12]
            if (i >= 12 && i <=14) {
                continue;
            }
            if (LPC_GPIOINT->IO0IntStatR & (1 << i)) {
                LPC_GPIOINT->IO0IntClr  |= (1 << i);
                LPC_GPIO0->FIOCLR       |= (1 << i);
                // If gpio interrupt handler exists call callback
                if (gpio_port0_interrupts[i] != NULL) {
                    gpio_port0_interrupts[i]->callback();
                }
            }
        }

        // Clear all active interrupts for GPIO port 2
        for (int i=0; i<14; i++) {
            if (LPC_GPIOINT->IO2IntStatR & (1 << i)) {
                LPC_GPIOINT->IO2IntClr  |= (1 << i);
                LPC_GPIO2->FIOCLR       |= (1 << i);
                // If gpio interrupt handler exists call callback
                if (gpio_port2_interrupts[i] != NULL) {
                    gpio_port2_interrupts[i]->callback();
                }
            }
        }
        
        Led0::getInstance().Toggle();
        Led1::getInstance().Toggle();
        Led2::getInstance().Toggle();
        Led3::getInstance().Toggle();

        // Unblock task
        long yield = 0;
        xSemaphoreGiveFromISR(EINT2_Sem, &yield);
        portYIELD_FROM_ISR(yield);
    }

    void EINT3_IRQHandler()
    {
        // Clear all active interrupts for GPIO port 0
        for (int i=0; i<31; i++) {
            // Reserved bits [14:12]
            if (i >= 12 && i <=14) {
                continue;
            }
            if (LPC_GPIOINT->IO0IntStatR & (1 << i)) {
                LPC_GPIOINT->IO0IntClr  |= (1 << i);
                LPC_GPIO0->FIOCLR       |= (1 << i);
                // If gpio interrupt handler exists call callback
                if (gpio_port0_interrupts[i] != NULL) {
                    gpio_port0_interrupts[i]->callback();
                }
            }
        }

        // Clear all active interrupts for GPIO port 2
        for (int i=0; i<14; i++) {
            if (LPC_GPIOINT->IO2IntStatR & (1 << i)) {
                LPC_GPIOINT->IO2IntClr  |= (1 << i);
                LPC_GPIO2->FIOCLR       |= (1 << i);
                // If gpio interrupt handler exists call callback
                if (gpio_port2_interrupts[i] != NULL) {
                    gpio_port2_interrupts[i]->callback();
                }
            }
        }
        
        Led0::getInstance().Toggle();
        Led1::getInstance().Toggle();
        Led2::getInstance().Toggle();
        Led3::getInstance().Toggle();

        // Unblock task
        long yield = 0;
        xSemaphoreGiveFromISR(EINT3_Sem, &yield);
        portYIELD_FROM_ISR(yield);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

GpioInterrupt::GpioInterrupt(external_interrupt_t eint, 
                                                interrupt_edge_t    edge,
                                                gpio_port_t         port, 
                                                gpio_pin_t          pin,
                                                void_function_ptr_t callback) : 
                                                GpioOutput(port, pin)
{
    // Store configuration
    GpioIntr.eint     = eint;
    GpioIntr.edge     = edge;
    GpioIntr.port     = port;
    GpioIntr.pin      = pin;
    GpioIntr.callback = callback;

    // Store callback
    switch (port)
    {
        case GPIO_PORT0: gpio_port0_interrupts[pin] = &GpioIntr; break;
        case GPIO_PORT2: gpio_port2_interrupts[pin] = &GpioIntr; break;
        default: 
            printf("Interrupt on GPIO ports other than 0 and 2 are not supported.\n");
            exit(EXIT_FAILURE);
    }

    // Initialize
    InitializeInterrupt();

    printf("GPIO External Interrupt initialized on port: %i pin: %i\n", port, pin);
}

GpioInterrupt::GpioInterrupt(gpio_interrupt_t gpio_interrupt) : 
                            GpioOutput(gpio_interrupt.port, gpio_interrupt.pin)
{
    // Store configuration
    GpioIntr = gpio_interrupt;

    // Store callback
    switch (GpioIntr.port)
    {
        case GPIO_PORT0: gpio_port0_interrupts[GpioIntr.pin] = &GpioIntr; break;
        case GPIO_PORT2: gpio_port2_interrupts[GpioIntr.pin] = &GpioIntr; break;
        default: 
            printf("Interrupt on GPIO ports other than 0 and 2 are not supported.\n");
            exit(EXIT_FAILURE);
    }

    // Initialize
    InitializeInterrupt();

    printf("GPIO External Interrupt initialized on port: %i pin: %i\n", 
                                                gpio_interrupt.port, 
                                                gpio_interrupt.pin);
}

void GpioInterrupt::RegisterCallback(void_function_ptr_t callback)
{
    GpioIntr.callback = callback;
}

void GpioInterrupt::InitializeInterrupt()
{
    // Disable interrupt before configuration
    switch (GpioIntr.eint)
    {
        case EINT0: NVIC_DisableIRQ(EINT0_IRQn); break;
        case EINT1: NVIC_DisableIRQ(EINT1_IRQn); break;
        case EINT2: NVIC_DisableIRQ(EINT2_IRQn); break;
        case EINT3: NVIC_DisableIRQ(EINT3_IRQn); break;
    }

    // Edge sensitive
    LPC_SC->EXTMODE  |= (1 << GpioIntr.pin);
    // Rising edge
    LPC_SC->EXTPOLAR |= (1 << GpioIntr.pin);
    // Clear EINT bit to start
    LPC_SC->EXTINT   |= (1 << GpioIntr.pin);

    // Turn on rising edge interrupt for pin
    switch (GpioIntr.port)
    {
        case GPIO_PORT0:

            if (GpioIntr.edge == RISING) {
                LPC_GPIOINT->IO0IntEnR = (1 << GpioIntr.pin);
                LPC_GPIOINT->IO0IntEnF = 0;
            }
            else {
                LPC_GPIOINT->IO0IntEnR = 0;
                LPC_GPIOINT->IO0IntEnF = (1 << GpioIntr.pin);
            }
            break;

        case GPIO_PORT2:

            if (GpioIntr.edge == RISING) {
                LPC_GPIOINT->IO2IntEnR = (1 << GpioIntr.pin);
                LPC_GPIOINT->IO2IntEnF = 0;
            }
            else {
                LPC_GPIOINT->IO2IntEnR = 0;
                LPC_GPIOINT->IO2IntEnF = (1 << GpioIntr.pin);
            }
            break;

        default:
            printf("Interrupt on GPIO ports other than 0 and 2 are not supported.\n");
            exit(EXIT_FAILURE);
    }

    // Enable interrupt
    switch (GpioIntr.eint)
    {
        case EINT0: NVIC_EnableIRQ(EINT0_IRQn); break;
        case EINT1: NVIC_EnableIRQ(EINT1_IRQn); break;
        case EINT2: NVIC_EnableIRQ(EINT2_IRQn); break;
        case EINT3: NVIC_EnableIRQ(EINT3_IRQn); break;
    }
}