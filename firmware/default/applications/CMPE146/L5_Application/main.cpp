#include "tasks.hpp" 									// terminalTask
#include "L5_Application/labs/Switch2LedTask.cpp"       // Lab1
#include "L5_Application/labs/UartTask.cpp"				// Lab2
#include "L5_Application/labs/SpiTask.cpp"				// Lab3
// #include "interrupt.hpp"				// Lab4
// #include ".hpp"							// Lab5
// #include "orientation.cpp"				// Lab6
// #include "producer_consumer.cpp"		// Lab7
// #include "infrared.cpp"
// #include "bluetooth.cpp"
// #include "motor.hpp"


int main(void)
{
	/******************************************************************************************************/
	/* Lab 1 : GPIO : DONE */
	printf("Starting Lab 1...\n");
	scheduler_add_task(new Switch2LedTask(PRIORITY_LOW));

	/******************************************************************************************************/

	/* Lab 2 : UART : DONE */
	// scheduler_add_task(new UartSendTask(PRIORITY_LOW, UART_PORT2));
	// scheduler_add_task(new UartEchoTask(PRIORITY_LOW, UART_PORT3));

	/******************************************************************************************************/

	/* Lab 3 : SPI : DONE (WORKING ON EXTRA CREDIT) */
	// scheduler_add_task(new spiSend_task(PRIORITY_LOW));

	/******************************************************************************************************/

	/* Lab 4 : I2C : INCOMPLETE */
	// scheduler_add_task(new (PRIORITY_LOW));
	// scheduler_add_task(new (PRIORITY_LOW));

	/******************************************************************************************************/

	/* Lab 5 : INTERRUPTS : Not working but mostly complete */
	// scheduler_add_task(new InterruptLab(PRIORITY_HIGH));
	// scheduler_add_task(new sw0_callback());
	// scheduler_add_task(new sw1_callback());

	/******************************************************************************************************/

	/* Lab 6 : ORIENTATION : DONE */

	// scheduler_add_task(new orient_compute(PRIORITY_LOW));
	// scheduler_add_task(new orient_process(PRIORITY_LOW));
	// scheduler_add_task(new terminalTask(PRIORITY_HIGH));

	/******************************************************************************************************/

    /* Lab 7 : PRODUCER/CONSUMER/WATCHDOG : Finished everything except reading files from SD card */

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
