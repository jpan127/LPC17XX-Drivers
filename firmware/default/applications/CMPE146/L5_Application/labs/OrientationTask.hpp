#pragma once
#include <stdio.h>
#include <LPC17xx.h>
#include <FreeRTOS.h>
#include <scheduler_task.hpp>
#include <task.h>
#include <queue.h>
#include "L5_Application/drivers/buttons.hpp"
#include "L5_Application/drivers/led.hpp"
#include "L5_Application/drivers/accelerometer.hpp"

typedef enum { UP, DOWN, LEFT, RIGHT } 	orientation_t;
typedef enum { SHARED_QUEUE_HANDLE } 	shared_handle_id_t;

typedef struct
{
	orientation_t x;
	orientation_t y;
} __attribute__((packed)) orientation_xy_t;

typedef struct 
{ 
	int16_t				x;
	int16_t				y;
	int16_t				z;
	orientation_xy_t 	orientation;
} __attribute__((packed)) axis_position_t;

// When OrientationProcessTask is higher priority than OrientationGetTask,
// the after send message gets printed after the queue receives.  Otherwise it gets printed
// immediately after the before send message.

// Block time is used to block the task until the queue receives and relinquish CPU control
// allowing other tasks to hold the CPU until unblocked.

// Using zero block time would cause the task to busy wait until it is forced to context switch
// after its allocated time.

class OrientationGetTask : public scheduler_task
{
public:

	OrientationGetTask(uint8_t priority) : scheduler_task("OrientationGetTask", 2048, priority)
	{
		Queue = xQueueCreate(1, sizeof(Axis));
		addSharedObject(SHARED_QUEUE_HANDLE, Queue);
	}

	bool run(void* p)
	{
		if ( Button0::getInstance().IsPressed() ) 
		{
			Axis.x = Accelerometer::getInstance().GetX();
			Axis.y = Accelerometer::getInstance().GetY();
			Axis.z = Accelerometer::getInstance().GetZ();

			Axis.orientation.x = (Axis.x > 0) ? (LEFT) : (RIGHT);
			Axis.orientation.y = (Axis.y > 0) ? (UP)   : (DOWN);

			puts("[OrientationGetTask] Before send.");

			if ( !(xQueueSend(Queue, &Axis, portMAX_DELAY)) )
			{
				puts("[OrientationGetTask] Failed to send.");
			}
			
			puts("[OrientationGetTask] After send.");
		}

		vTaskDelay(100);
		return true;
	}

private:

	QueueHandle_t 	Queue;
	axis_position_t	Axis;
};


///////////////////////////////////////////////////////////////////////////////////////////////////


class OrientationProcessTask : public scheduler_task
{
public:

	OrientationProcessTask (uint8_t priority) : scheduler_task("OrientationProcessTask", 2048, priority)
	{
		Queue = NULL;
	}

	bool run(void* p)
	{
		Queue = getSharedObject(SHARED_QUEUE_HANDLE);

		if ( !(xQueueReceive(Queue, &Axis, portMAX_DELAY)) )
		{
			puts("[OrientationProcessTask] Failed to receive.");
		}

		printf("X: %i | Y: %i | Z: %i\n", Axis.x, Axis.y, Axis.z);

		switch (Axis.orientation.y)
		{
			case UP: 	printf("UP | "); 	break;
			case DOWN: 	printf("DOWN | "); 	break;
			default:						break;
		}
		switch (Axis.orientation.x)
		{
			case LEFT:	printf("LEFT\n"); 	break;
			case RIGHT: printf("RIGHT\n"); 	break;
			default:						break;
		}

		LightUpLeds();

		return true;
	}

	void LightUpLeds()
	{
		Led0::getInstance().ClearLed();
		Led1::getInstance().ClearLed();
		Led2::getInstance().ClearLed();
		Led3::getInstance().ClearLed();

		(Axis.orientation.x == LEFT) ? (Led0::getInstance().SetLed()) : (Led3::getInstance().SetLed());
		(Axis.orientation.y == UP)   ? (Led1::getInstance().SetLed()) : (Led2::getInstance().SetLed());
	}

private:

	QueueHandle_t 	Queue;
	axis_position_t	Axis;
};



