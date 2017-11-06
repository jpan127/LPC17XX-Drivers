#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <scheduler_task.hpp>
#include <io.hpp>
#include <tasks.hpp>
#include <event_groups.h>
#include <stop_watch.hpp>


// Helper macros
#define DELAY(ms)       (vTaskDelay(ms / portTICK_PERIOD_MS))
#define US_TO_MS(us)    ((float)us / 1000000)

// Bits
#define BIT1            (1 << 1)
#define BIT2            (1 << 2)
#define BITS_1_2        ((1 << 1) | (1 << 2))

// Constants
#define SAMPLE_SIZE     (100)

// Enums
typedef enum    { QUEUE_ID }        shared_queue_id_t;
typedef struct  { float average; }  shared_data_t;

// Shared event group between ProducerTask, ConsumerTask, and WatchdogTask
static EventGroupHandle_t WatchdogEventGroup = xEventGroupCreate();



class ProducerTask : public scheduler_task
{
public:

    ProducerTask(uint8_t priority) : scheduler_task("ProducerTask", 2048, priority)
    {
        // Clear everything
        SampleNum         = 0;
        LightData.average = 0;
        memset(Samples, 0, sizeof(float) * SAMPLE_SIZE);

        // Create a queue, and add shared object
        QueueHandle_t my_queue = xQueueCreate(1, sizeof(shared_data_t));
        addSharedObject(QUEUE_ID, my_queue);
    }

    // Grabs a sample every 1ms, after 100, computes the average and sends it to shared queue
    bool run(void *p)
    {
        // Read sensor
        Samples[SampleNum++] = LS.getRawValue();

        // Send to queue
        if (SampleNum == SAMPLE_SIZE) 
        {
            SampleNum = 0;
            ComputeAverage();
            xQueueSend( getSharedObject(QUEUE_ID), &LightData, portMAX_DELAY );
        }

        DELAY(1);
        EventHandler();
        return true;
    }

    // Calculates the average and sets the value in the shared struct
    float ComputeAverage()
    {
        LightData.average = 0;
        for (int i=0; i<SAMPLE_SIZE; i++) {
            LightData.average += Samples[i];
        }
        LightData.average = LightData.average / SAMPLE_SIZE;

        return LightData.average;
    }

    // Sets the bit for this task in the eventgroup
    void EventHandler()
    {
        // Set bit 1
        EventBits_t bits = xEventGroupSetBits( WatchdogEventGroup, BIT1 );

        // Log error if not set correctly
        if ((bits & BIT1) != BIT1) {
            printf("[ProducerTask] Bit 1 failed to set.\n");
        }
    }

private:
    float           Samples[SAMPLE_SIZE];
    int             SampleNum;
    shared_data_t   LightData;
};


///////////////////////////////////////////////////////////////////////////////////////////////////


class ConsumerTask : public scheduler_task
{
public:

    ConsumerTask(uint8_t priority) : scheduler_task("ConsumerTask", 2048, priority)
    {
        // Clear everything
        memset(Averages, 0, sizeof(float) * 10);
        memset(Times,    0, sizeof(float) * 10);
        Counter = 0;
        LightData.average = 0;

        // Start timer
        Timer.start();
    }

    bool run(void *p)
    {
        // Fetch queue
        QueueHandle_t q = getSharedObject(QUEUE_ID);
        if ( xQueueReceive(q, &LightData, portMAX_DELAY) )
        {
            /* Empty */
        }

        HandleData();
        EventHandler();
        return true;
    }

    void HandleData()
    {
        // Store average and time of arrival
        Averages[Counter] = LightData.average;
        Times[Counter++]  = US_TO_MS(Timer.getElapsedTime());

        // After 10 averages (1000ms), write to file
        if (Counter == 10) 
        {
            Counter = 0;
            WriteToFile();
        }
    }

    // Sets the bit for this task in the eventgroup
    void EventHandler()
    {
        // Set bit 2
        EventBits_t bits = xEventGroupSetBits( WatchdogEventGroup, BIT2 );

        // Log error if not set correctly
        if ((bits & BIT2) != BIT2) {
            printf("[ConsumerTask] Bit 2 failed to set.\n");
        }
    }

    // Writes time and average to file, values verified, file output not verified though
    void WriteToFile()
    {
        static int count = 0;

        FILE *file = fopen("1:sensor.txt", "a");

        if (file)
        {            
            for (int i=0; i<10; i++) 
            {
                printf(       "[%i] %4.3f : %4.3f\n", count, Times[i], Averages[i]);
                fprintf(file, "[%i] %4.3f : %4.3f\n", count, Times[i], Averages[i]);
            }

            fclose(file);
        }

        FILE *csv = fopen("1:sensor.csv", "a");

        if (csv)
        {
            for (int i=0; i<10; i++)
            {
                fprintf(csv, "%i,%4.3f,%4.3f\n", count, Times[i], Averages[i]);
            }
            fclose(csv);
        }

        count++;
    }

private:
    float                   Averages[10];
    float                   Times[10];
    int                     Counter;
    shared_data_t           LightData;
    MicroSecondStopWatch    Timer;
};


///////////////////////////////////////////////////////////////////////////////////////////////////


class WatchdogTask : public scheduler_task
{
public:

    WatchdogTask(uint8_t priority) : scheduler_task("WatchdogTask", 2048, priority)
    {
        // Start timer
        Timer.start();
    }

    // Check eventgroup bits
    bool run(void *p)
    {
        // Second Counter
        static int second = 0;

        // Check event bits
        HandleEventGroup();

        // Increment second, every minute monitor cpu usage
        if (++second == 60) 
        {
            second = 0;
            MonitorCpuUsage();
        }

        return true;
    }

    void HandleEventGroup()
    {
        // Constant amount to wait for bits = 1 second
        const TickType_t xTicksToWait = 1000 / portTICK_PERIOD_MS;

        // Get time before xEventGroupWaitBits
        float start_time = (float)Timer.getElapsedTime() / 1000000;

        // Clear on exit, and wait for all bits
        EventBits_t bits = xEventGroupWaitBits( WatchdogEventGroup,
                                                BIT1 | BIT2, 
                                                pdTRUE, 
                                                pdTRUE, 
                                                xTicksToWait);

        // Get time after xEventGroupWaitBits
        float end_time = (float)Timer.getElapsedTime() / 1000000;

        // Check returns bits from xEventGroupWaitBits
        switch (bits & BITS_1_2)
        {
            case BITS_1_2:
            {
                // Empty
                break;
            }
            case BIT1:
            {
                FILE *file = fopen("1:stuck.txt", "a");
                if (file)
                {
                    printf(       "[%4.3f] Stuck: Producer \n", end_time);
                    fprintf(file, "[%4.3f] Stuck: Producer \n", end_time);
                    fclose(file);             
                }
                break;
            }
            case BIT2:
            {
                FILE *file = fopen("1:stuck.txt", "a");
                if (file)
                {
                    printf(       "[%4.3f] Stuck: Consumer \n", end_time);
                    fprintf(file, "[%4.3f] Stuck: Consumer \n", end_time);
                    fclose(file);
                }
                break;
            }
            default:
            {
                FILE *file = fopen("1:stuck.txt", "a");
                if (file)
                {
                    printf(       "[%4.3f] Stuck: Producer and Consumer \n", end_time);
                    fprintf(file, "[%4.3f] Stuck: Producer and Consumer \n", end_time);
                    fclose(file);
                }
                break;
            }
        }
        
        // Time remaining in milliseconds if xEventGroupWaitBits returned early
        float time_left = (1 - (end_time - start_time)) * 1000;
        DELAY(time_left);
    }

    // Prints out CPU usage information
    void MonitorCpuUsage()
    {
        char buffer[280];
        sys_get_mem_info_str(buffer);

        printf("%s\n", buffer);

        FILE *file = fopen("1:cpu.txt", "a");
        if (file)
        {
            printf("%s\n", buffer);
            fprintf(file,  buffer);
            fclose(file);
        }
    }

private:
    MicroSecondStopWatch    Timer;
};