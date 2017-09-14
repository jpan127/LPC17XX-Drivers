#include "tasks.hpp" 									// terminalTask
#include "L5_Application/labs/Switch2LedTask.cpp"       // Lab1
#include "L5_Application/labs/UartTask.hpp"				// Lab2
#include "L5_Application/labs/AT45QueryTask.cpp"		// Lab3
#include "L5_Application/labs/GpioInterruptTask.hpp"	// Lab4
// #include "L5_Application/labs/.hpp"						// Lab5
// #include "L5_Application/labs/orientation.cpp"			// Lab6
// #include "L5_Application/labs/producer_consumer.cpp"	// Lab7
// #include "infrared.cpp"
// #include "bluetooth.cpp"
// #include "motor.hpp"


int main(void)
{
	/******************************************************************************************************/
	/* Lab 1 | GPIO | VERIFIED */
	// printf("Starting Lab 1...\n");
	// scheduler_add_task(new Switch2LedTask(PRIORITY_LOW));

	/******************************************************************************************************/

	/* Lab 2 | UART | VERIFIED */
	// printf("Starting Lab 2...\n");
	// scheduler_add_task(new UartTask(PRIORITY_MEDIUM, UART_PORT));

	/******************************************************************************************************/

	/* Lab 3 | SPI | DONE (WORKING ON EXTRA CREDIT) */
	// scheduler_add_task(new AT45QueryTask(PRIORITY_LOW));

	/******************************************************************************************************/

	/* Lab 4 | I2C | INCOMPLETE */
	// scheduler_add_task(new (PRIORITY_LOW));
	// scheduler_add_task(new (PRIORITY_LOW));

	/******************************************************************************************************/

	/* Lab 5 | INTERRUPTS | Not working but mostly complete */
	gpio_interrupt_t gpio_interrupt;
    gpio_interrupt.eint 	= EINT3;
    gpio_interrupt.edge 	= RISING;
    gpio_interrupt.port 	= GPIO_PORT2;
    gpio_interrupt.pin  	= 7;
    gpio_interrupt.callback = Callback2;
	scheduler_add_task(new GpioInterruptTask(PRIORITY_LOW, gpio_interrupt));

	xTaskCreate(&InterruptCallbackTask1, "InterruptCallbackTask1", 2048, NULL, PRIORITY_LOW, NULL);
	xTaskCreate(&InterruptCallbackTask2, "InterruptCallbackTask2", 2048, NULL, PRIORITY_LOW, NULL);

	/******************************************************************************************************/

	/* Lab 6 | ORIENTATION | DONE */

	// scheduler_add_task(new orient_compute(PRIORITY_LOW));
	// scheduler_add_task(new orient_process(PRIORITY_LOW));
	// scheduler_add_task(new terminalTask(PRIORITY_HIGH));

	/******************************************************************************************************/

    /* Lab 7 | PRODUCER/CONSUMER/WATCHDOG | Finished everything except reading files from SD card */

	// scheduler_add_task(new producer_task(PRIORITY_MEDIUM));
	// scheduler_add_task(new consumer_task(PRIORITY_MEDIUM));
	// scheduler_add_task(new watchdog_task(PRIORITY_HIGH));
	// scheduler_add_task(new terminalTask(PRIORITY_HIGH));

	/******************************************************************************************************/

	/* Infra-red test */

	// scheduler_add_task(new infrared_task(PRIORITY_LOW));

	/******************************************************************************************************/

	/* Bluetooth test */

	// scheduler_add_task(new BluetoothTask(PRIORITY_LOW));

	/******************************************************************************************************/

	/* Motor Driver */

	// scheduler_add_task(new MotorTask(PRIORITY_LOW));

	scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    scheduler_start();
    return -1;
}
