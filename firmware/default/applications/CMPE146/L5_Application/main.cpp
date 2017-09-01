#include "tasks.hpp" 					// terminalTask
// #include "switch2led_task.cpp"          // Lab1
#include "UartTask.cpp"
// #include "spi.cpp"
// #include "interrupt.hpp"
// #include "orientation.cpp"
// #include "producer_consumer.cpp"
// #include "infrared.cpp"
// #include "bluetooth.cpp"
// #include "motor.hpp"


int main(void)
{
	/******************************************************************************************************/

	/* Lab 1 : GPIO : DONE */
	// scheduler_add_task(new Switch2LedTask(PRIORITY_LOW));
	// scheduler_add_task(new terminalTask(PRIORITY_HIGH));

	/******************************************************************************************************/

	/* Lab 2 : UART : DONE */
	scheduler_add_task(new UartSendTask(PRIORITY_LOW, PORT2));
	scheduler_add_task(new UartEchoTask(PRIORITY_LOW, PORT3));

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

    scheduler_start();
    return -1;
}