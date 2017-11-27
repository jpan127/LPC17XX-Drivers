#include "tasks.hpp"                                    // terminalTask
// #include "L5_Application/labs/Switch2LedTask.cpp"       // Lab1
// #include "L5_Application/labs/UartTask.hpp"             // Lab2
// #include "L5_Application/labs/AT45QueryTask.cpp"        // Lab3
// #include "L5_Application/labs/GpioInterruptTask.hpp"    // Lab4
// #include "L5_Application/labs/I2CSlaveTask.hpp"         // Lab5
// #include "L5_Application/labs/I2CMasterTask.hpp"        // Lab5
// #include "L5_Application/labs/OrientationTask.hpp"      // Lab6
// #include "L5_Application/labs/PriorityTasks.hpp"        // Lab6
// #include "L5_Application/labs/ProducerConsumerTasks.cpp" // Lab7
#include "MP3Task.hpp"


int main(void)
{    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /* Lab 1 | GPIO | VERIFIED */

    // scheduler_add_task(new Switch2LedTask(PRIORITY_LOW));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Lab 2 | UART | VERIFIED */

    // printf("Starting Lab 2...\n");
    // scheduler_add_task(new UartTask(PRIORITY_MEDIUM, UART_PORT));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Lab 3 | SPI | VERIFIED */

    // scheduler_add_task(new AT45QueryTask(PRIORITY_LOW));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Lab 4 | I2C | VERIFIED */

    // scheduler_add_task(new I2CSlaveTask(PRIORITY_MEDIUM, I2C_PORT2));
    // scheduler_add_task(new I2CMasterTask(PRIORITY_MEDIUM));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Lab 5 | INTERRUPTS | VERIFIED */

    // gpio_interrupt_t *gpio_interrupt1 = new gpio_interrupt_t;
    // gpio_interrupt1->eint      = EINT3;
    // gpio_interrupt1->edge      = RISING;
    // gpio_interrupt1->port      = GPIO_PORT2;
    // gpio_interrupt1->pin       = 6;
    // gpio_interrupt1->callback = Callback1;

    // gpio_interrupt_t *gpio_interrupt2 = new gpio_interrupt_t;
    // gpio_interrupt2->eint      = EINT3;
    // gpio_interrupt2->edge      = RISING;
    // gpio_interrupt2->port      = GPIO_PORT2;
    // gpio_interrupt2->pin       = 7;
    // gpio_interrupt2->callback = Callback2;
    // scheduler_add_task(new GpioInterruptTask(PRIORITY_LOW, gpio_interrupt1, gpio_interrupt2));

    // xTaskCreate(&InterruptCallbackTask1, "InterruptCallbackTask1", 2048, NULL, PRIORITY_LOW, NULL);
    // xTaskCreate(&InterruptCallbackTask2, "InterruptCallbackTask2", 2048, NULL, PRIORITY_LOW, NULL);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Lab 6 | ORIENTATION | VERIFIED */

    // Part 1
    // xTaskCreate(&PriorityTaskHigh, "PriorityTaskHigh", 1024, NULL, PRIORITY_HIGH, NULL);
    // xTaskCreate(&PriorityTaskLow,  "PriorityTaskLow",  1024, NULL, PRIORITY_HIGH, NULL);

    // Part 2
    // scheduler_add_task(new OrientationGetTask(PRIORITY_LOW));
    // scheduler_add_task(new OrientationProcessTask(PRIORITY_MEDIUM));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    /* Lab 7 | PRODUCER/CONSUMER/WATCHDOG | Finished everything except reading files from SD card */

    // scheduler_add_task(new ProducerTask(PRIORITY_MEDIUM));
    // scheduler_add_task(new ConsumerTask(PRIORITY_MEDIUM));
    // scheduler_add_task(new WatchdogTask(PRIORITY_HIGH));

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    const vs1053b_gpio_init_t gpio_init = {
        .port_reset = GPIO_PORT0,
        .port_dreq  = GPIO_PORT0,
        .port_xcs   = GPIO_PORT0,
        .port_xdcs  = GPIO_PORT0,
        .pin_reset  = 30,
        .pin_dreq   = 29,
        .pin_xcs    = 1,
        .pin_xdcs   = 0,
    };

    // #define SPI_PORT    (SPI_PORT0)
    // #define SPI_CS      (16)
    // #define SPI_SCK     (15)
    // #define SPI_MISO    (17)
    // #define SPI_MOSI    (18)

    scheduler_add_task(new MP3Task(PRIORITY_MEDIUM, gpio_init));

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    scheduler_start();
    return -1;
}
