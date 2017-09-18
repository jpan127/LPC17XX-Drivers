#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include "scheduler_task.hpp"
#include "io.hpp"
#include "tasks.hpp"
#include "event_groups.h"
#include "stop_watch.hpp"
using namespace std;

/****************************************************************************************************************
 * 														Lab 7													*
 ***************************************************************************************************************/

typedef enum 	{ shared_queue_id } shared_id_t;
typedef struct 	{ float average; } shared_struct_t;

#define BIT1 		(1 << 1)
#define BIT2 		(1 << 2)
#define BITS_1_2	((1 << 1) | (1 << 2))

static const EventGroupHandle_t watchdog_eventgroup = xEventGroupCreate();

class producer_task : public scheduler_task
{
public:

	producer_task(uint8_t priority) : scheduler_task("producer_task", 2048, priority)
	{
		sample_num 			= 0;
		my_struct.average 	= 0;
		QueueHandle_t my_queue = xQueueCreate(1, sizeof(shared_struct_t));
		addSharedObject(shared_queue_id, my_queue);
	}

	// Grabs a sample every 1ms, after 100, computes the average and sends it to shared queue
	bool run(void *p)
	{
		samples[sample_num++] = LS.getRawValue();

		if (sample_num == 100) {
			sample_num = 0;
			get_average();
			xQueueSend( getSharedObject(shared_queue_id), &my_struct, portMAX_DELAY );
		}

		vTaskDelay(10);
		handle_eventgroup();
		return true;
	}

	// Calculates the average and sets the value in the shared struct
	float get_average()
	{
		my_struct.average = 0;
		for (int i=0; i<100; i++) {
			my_struct.average += samples[i];
		}
		my_struct.average = my_struct.average / 100;

		return my_struct.average;
	}

	// Sets the bit for this task in the eventgroup
	void handle_eventgroup()
	{
		EventBits_t bits;

		// Set bit 2
		bits = xEventGroupSetBits( watchdog_eventgroup, BIT1 );

		// Check if set
		if ((bits & BIT1) == BIT1) {
			 // Bit is set
		}
		else {
			// Bit was cleared
			cout << "Bit 1 was not set." << endl;
		}
	}

private:
	float 			samples[100];
	int				sample_num;
	shared_struct_t my_struct;
};


/******************************************************************************************************/


class consumer_task : public scheduler_task
{
public:

	consumer_task(uint8_t priority) : scheduler_task("consumer_task", 2048, priority)
	{
		counter = 0;
		my_struct.average = 0;
		swatch.start();
	}

	bool run(void *p)
	{
		QueueHandle_t my_queue = getSharedObject(shared_queue_id);			// Fetch queue
		if ( xQueueReceive(my_queue, &my_struct, portMAX_DELAY) )
		{
			/* Empty */
		}

		handle_queue();
		handle_eventgroup();
		return true;
	}

	void handle_queue()
	{
		averages[counter] 	= my_struct.average;
		times[counter++]	= (float)swatch.getElapsedTime() / 1000000;

		if (counter == 10) {												// After 10 averages, write to file
			counter = 0;
			write_to_file();
		}
	}

	// Sets the bit for this task in the eventgroup
	void handle_eventgroup()
	{
		EventBits_t bits;

		// Set bit 2
		bits = xEventGroupSetBits( watchdog_eventgroup, BIT2 );

		// Check if set
		if ((bits & BIT2) == BIT2) {
			 // Bit is set
		}
		else {
			// Bit was cleared
//			cout << "Bit 2 was not set." << endl;
		}
	}

	// Writes time and average to file, values verified, file output not verified though
	void write_to_file()
	{
		static int count = 0;

		FILE* ins = fopen("sensor.txt", "a");										// append

		for (int i=0; i<10; i++) {
			cout << "[" << count++ << "] " << fixed << setprecision(3) << times[i] << ", " << averages[i] << endl;
			fprintf(ins, "[%i] %4.3f, %4.3f\n", count, times[i], averages[i]);
		}

		fclose(ins);
	}

private:
	float					averages[10];
	float					times[10];
	int 					counter;
	shared_struct_t 		my_struct;
	MicroSecondStopWatch 	swatch;
};


/******************************************************************************************************/


class watchdog_task : public scheduler_task
{
public:

	watchdog_task(uint8_t priority) : scheduler_task("watchdog_task", 2048, priority)
	{
		swatch.start();
	}

	// Check eventgroup bits
	bool run(void *p)
	{
		// Second counter
		static int second = 0;

		// Get time before xEventGroupWaitBits
		float start_time = (float)swatch.getElapsedTime() / 1000000;

		EventBits_t bits;
		const TickType_t xTicksToWait = 1000 / portTICK_PERIOD_MS;
		// Clear on exit, and wait for all bits
		bits = xEventGroupWaitBits(watchdog_eventgroup, BIT1 | BIT2, pdTRUE, pdTRUE, xTicksToWait);

		// Get time after xEventGroupWaitBits
		float end_time = (float)swatch.getElapsedTime() / 1000000;

		// Check returns bits from xEventGroupWaitBits
		if ((bits & BITS_1_2) == BITS_1_2)
		{
			// Time remaining in milliseconds if xEventGroupWaitBits returned early
			float time_left = (1 - (end_time - start_time)) * 1000;

			vTaskDelay(time_left);
		}
		else if ((bits & BIT1) == BIT1)
		{
			FILE *stuck_log = fopen("stuck.txt", "a");
			fprintf(stuck_log, "[%4.3f] Stuck: Producer \n", end_time);
			printf("[%4.3f] Stuck: Producer \n", end_time);
			fclose(stuck_log);
		}
		else if ((bits & BIT2) == BIT2)
		{
			FILE *stuck_log = fopen("stuck.txt", "a");
			fprintf(stuck_log, "[%4.3f] Stuck: Consumer \n", end_time);
			printf("[%4.3f] Stuck: Consumer \n", end_time);
			fclose(stuck_log);
		}
		else
		{
			FILE *stuck_log = fopen("stuck.txt", "a");
			fprintf(stuck_log, "[%4.3f] Stuck: Producer and Consumer \n", end_time);
			printf("[%4.3f] Stuck: Producer and Consumer \n", end_time);
			fclose(stuck_log);
		}

		// Increment second, every minute monitor cpu usage
		second++;
		if (second == 60) {
			second = 0;
			monitor_cpu_usage();
		}
		return true;
	}

	// Prints some junk on the last line, not sure how to get rid of it
	// maybe only print ascii? but doesn't fix the problem
	void monitor_cpu_usage()
	{
	    char buffer[280];
	    sys_get_mem_info_str(buffer);

	    for (int i=0; i<280; i++) {
	    	cout << buffer[i];
	    }
	    cout << endl;

	    FILE *cpu_log = fopen("cpu.txt", "a");
	    fprintf(cpu_log, buffer);
	    fclose(cpu_log);
	}

private:
	MicroSecondStopWatch 	swatch;
};
