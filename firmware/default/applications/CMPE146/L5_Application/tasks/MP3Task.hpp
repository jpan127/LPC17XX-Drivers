#pragma once
#include <ff.h>
#include <ssp0.h>
#include <scheduler_task.hpp>
#include "L5_Application/drivers/vs1053b.hpp"
#include "L5_Application/drivers/buttons.hpp"

#define MP3_SEGMENT_SIZE (1024)

typedef enum
{
    IDLE,
    PLAY,
    STOP,
    FORWARD,
    BACKWARD
} mpe_next_state_E;

typedef struct
{
    bool cancel_requested;
    mpe_next_state_E next_state;
} MP3_status_S;

static const vs1053b_gpio_init_t gpio_init = {
    .port_reset = GPIO_PORT0,
    .port_dreq  = GPIO_PORT0,
    .port_xcs   = GPIO_PORT0,
    .port_xdcs  = GPIO_PORT0,
    .pin_reset  = 30,
    .pin_dreq   = 19,
    .pin_xcs    = 29,
    .pin_xdcs   = 0,
};

// File pointer to an mp3 file off an SD card
static FIL          mp3_file;
// VS1053b object which handles the device drivers
static VS1053b      MP3Player(gpio_init);
// Buffer for an MP3 segment to send to the device
static TCHAR        Buffer[MP3_SEGMENT_SIZE];
// Status information of the MP3 player
static MP3_status_S Status;

static void HandleStateLogic()
{
    static uint32_t segment_counter = 0;
    static bool last_segment = false;
    static UINT current_segment_size = 0;
    static vs1053b_transfer_status_E status;
    FRESULT result;

    switch (Status.next_state)
    {
        case IDLE:
            // Do nothing
            break;
        case PLAY:
            // Not currently playing, set up playback
            if (!MP3Player.IsPlaying())
            {
                // Reset values to default
                last_segment = false;
                segment_counter = 0;
                // Open mp3 file
                result = f_open(&mp3_file, "1:Halloween_V.mp3", FA_OPEN_EXISTING | FA_READ);
                if (result != FR_OK)
                {
                    printf("[MP3Task] mp3 file failed to open. Error: %d\n", result);
                    Status.next_state = IDLE;
                    break;
                }
                else
                {
                    printf("Halloween_V.mp3 Successfully opened.\n");
                }
            }

            // Read a segment at a time before sending to device
            current_segment_size = 0;
            result = f_read(&mp3_file, Buffer, MP3_SEGMENT_SIZE, &current_segment_size);
            if (result != FR_OK)
            {
                printf("[MP3Task] mp3 file failed to read. Error: %d\n", result);
            }
            // Set flag if last segment
            if (current_segment_size < MP3_SEGMENT_SIZE)
            {
                last_segment = true;
            }

            // Send segment to device
            status = MP3Player.PlaySegment((uint8_t*)Buffer, current_segment_size, last_segment);

            printf("[MP3Task] Played segment %lu with %d bytes.\n", segment_counter++, current_segment_size);

            // Handle transfer status
            if (TRANSFER_CANCELLED == status)
            {
                if (Status.cancel_requested)
                {
                    printf("[MP3Task] Cancel successful.\n");
                }
                else
                {
                    printf("[MP3Task] Playback cancelled but never requested!\n");
                }
                // Set last segment flag to run clean up
                last_segment = true;
                // Reset flags
                Status.cancel_requested = false;
            }
            else if (TRANSFER_FAILED == status)
            {
                printf("[MP3Task] Segment transfer failed. Stopping playback.\n");
                Status.next_state = IDLE;
            }

            ////////////////////////////
            // last_segment = true;
            ////////////////////////////

            // Clean up if last segment
            if (last_segment) 
            {
                result = f_close(&mp3_file);
                if (result != FR_OK)
                {
                    printf("[MP3Task] mp3 file failed to close!\n");
                }
                else
                {
                    printf("[MP3Task] mp3 file closed.\n");
                }
                Status.next_state = IDLE;
                
            }
            break;
        case STOP:
            // If stop requested, is currently playing, and not recently requested
            if (MP3Player.IsPlaying() && !Status.cancel_requested)
            {
                // Stop playback
                MP3Player.CancelDecoding();
                // Go back to playing to finish the last segment(s)
                Status.next_state = PLAY;
                // Set the flag for cancel request
                Status.cancel_requested = true;
            }
            // No need to cancel if not currently playing
            else
            {
                Status.next_state = IDLE;
            }
            break;
        case FORWARD:
            Status.next_state = IDLE;
            break;
        case BACKWARD:
            Status.next_state = IDLE;
            break;
    }
}

static void CheckButtons()
{
    // Only one button can be registered at a time
    if (Button0::getInstance().IsPressed())
    {
        Status.next_state = PLAY;
    }
    else if (Button1::getInstance().IsPressed())
    {
        Status.next_state = STOP;
    }
    else if (Button2::getInstance().IsPressed())
    {
        MP3Player.PrintDebugInformation();
        // next_state = FORWARD;
    }
    else if (Button3::getInstance().IsPressed())
    {
        // next_state = BACKWARD;
    }
}

void MP3Task(void *p)
{
    memset(Buffer, 0, sizeof(Buffer));
    ssp0_init(0);

    const uint8_t reset = 30;
    const uint8_t dreq  = 19;
    const uint8_t xcs   = 29;
    const uint8_t xdcs  = 0;

    // Set direction
    LPC_GPIO0->FIODIR |= (1 << reset);
    LPC_GPIO0->FIODIR &= ~(1 << dreq);
    LPC_GPIO0->FIODIR |= (1 << xcs);
    LPC_GPIO0->FIODIR |= (1 << xdcs);

    // Set initial state
    LPC_GPIO0->FIOSET |= (1 << xcs);
    LPC_GPIO0->FIOSET |= (1 << xdcs);
    LPC_GPIO0->FIOCLR |= (1 << reset);
    vTaskDelay(1 / portTICK_PERIOD_MS);
    LPC_GPIO0->FIOSET |= (1 << reset);
    while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );

    const uint16_t default_status = 0x4846; // line1, native spi, stream, reset, allow mpeg 1+2
    const uint16_t default_clock  = 0x6000;

    // CLOCKF
    while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
    LPC_GPIO0->FIOCLR |= (1 << xcs);
    ssp0_exchange_byte(0x02);
    ssp0_exchange_byte(0x3);
    ssp0_exchange_byte(default_status >> 8);
    ssp0_exchange_byte(default_status & 0xFF);
    LPC_GPIO0->FIOSET |= (1 << xcs);
    // MODE
    while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
    LPC_GPIO0->FIOCLR |= (1 << xcs);
    ssp0_exchange_byte(0x02);
    ssp0_exchange_byte(0x0);
    ssp0_exchange_byte(default_status >> 8);
    ssp0_exchange_byte(default_status & 0xFF);
    LPC_GPIO0->FIOSET |= (1 << xcs);

    // Wait for button trigger
    while ( !(Button0::getInstance().IsPressed()) );

    // Main loop
    while (1)
    {
        // Open mp3 file
        const char name[] = "1:track1.mp3"
        result = f_open(&mp3_file, name, FA_OPEN_EXISTING | FA_READ);
        if (result != FR_OK)
        {
            printf("[MP3Task] mp3 file failed to open. Error: %d\n", result);
            break;
        }
        else
        {
            printf("[MP3Task] %s successfully opened.\n", name);
        }
        // DECODE_TIME
        while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
        LPC_GPIO0->FIOCLR |= (1 << xcs);
        ssp0_exchange_byte(0x02);
        ssp0_exchange_byte(0x4);
        ssp0_exchange_byte(0);
        ssp0_exchange_byte(0);
        LPC_GPIO0->FIOSET |= (1 << xcs);
        // DECODE_TIME
        while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
        LPC_GPIO0->FIOCLR |= (1 << xcs);
        ssp0_exchange_byte(0x02);
        ssp0_exchange_byte(0x4);
        ssp0_exchange_byte(0);
        ssp0_exchange_byte(0);
        LPC_GPIO0->FIOSET |= (1 << xcs);

        // Read a segment at a time before sending to device
        current_segment_size = 0;

        // While reading max segment size, keep going
        while (current_segment_size < MP3_SEGMENT_SIZE)
        {
            // Read a segment
            result = f_read(&mp3_file, Buffer, MP3_SEGMENT_SIZE, &current_segment_size);
            if (result != FR_OK)
            {
                printf("[MP3Task] mp3 file failed to read. Error: %d\n", result);
                break;
            }

            uint32_t cycles    = current_segment_size / 32;
            uint16_t remainder = current_segment_size % 32;

            // Wait for DREQ
            while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
            // XDCS LOW
            LPC_GPIO0->FIOCLR |= (1 << xdcs);

            // Send bytes for each cycle
            for (uint32_t cycle=0; cycle<cycles; cycle++)
            {
                for (int byte=0; byte<32; byte++)
                {
                    ssp0_exchange_byte(data[cycle * 32 + byte]);
                }
                // Wait for DREQ
                while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
            }

            // Send remainder
            if (remainder > 0)
            {
                for (int byte=0; byte<remainder; byte++)
                {
                    // Cycles instead of cycle because it is after the last cycle
                    ssp0_exchange_byte(data[cycles * 32 + byte]);
                }
                // Wait for DREQ
                while ( !(LPC_GPIO0->FIOPIN & (1 << dreq)) );
            }

            // XDCS HIGH
            LPC_GPIO0->FIOSET |= (1 << xdcs);
        }

        // Close the damn file
        result = f_close(&mp3_file);
        if (result != FR_OK) printf("[MP3Task] mp3 file failed to close!\n");
        else                 printf("[MP3Task] mp3 file closed.\n");

        // Wait 10 seconds
        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
    }

    printf("[MP3Task] MP3 file failed so task is suspending...\n");
    vTaskSuspend(NULL);
}

// void MP3Task(void *p)
// {
//     // Initialize status struct
//     Status.cancel_requested = false;
//     Status.next_state = IDLE;

//     memset(Buffer, 0, sizeof(Buffer));
//     ssp0_init(0);
//     MP3Player.SystemInit();

//     while (1)
//     {
//         CheckButtons();
//         HandleStateLogic();
//         vTaskDelay(50 / portTICK_PERIOD_MS);
//     }
// }

// class MP3Task : public scheduler_task
// {
// public:

//     MP3Task(uint8_t priority, vs1053b_gpio_init_t gpio_init) : scheduler_task("MP3Task", 8196, priority), MP3Player(gpio_init)
//     {
//         // Initialize status struct
//         Status.cancel_requested = false;
//         Status.next_state = IDLE;

//         Buffer = (TCHAR*)malloc(sizeof(TCHAR) * MP3_SEGMENT_SIZE);
//     }

//     bool init()
//     {
//         MP3Player.SystemInit();
//         return true;
//     }

//     bool run(void *p)
//     {
//         CheckButtons();
//         HandleStateLogic();

//         vTaskDelay(50);
//         return true;
//     }

//     void HandleStateLogic()
//     {
//         static bool last_segment = false;
//         static UINT current_segment_size = 0;
//         static vs1053b_transfer_status_E status;
//         FRESULT result;

//         switch (Status.next_state)
//         {
//             case IDLE:
//                 // Do nothing
//                 break;
//             case PLAY:
//                 // Not currently playing, set up playback
//                 if (!MP3Player.IsPlaying())
//                 {
//                     // Reset values to default
//                     last_segment = false;
//                     // Open mp3 file
//                     result = f_open(&mp3_file, "1:Halloween_V.mp3", FA_OPEN_EXISTING | FA_READ);
//                     if (result != FR_OK)
//                     {
//                         printf("[MP3Task] mp3 file failed to open. Error: %d\n", result);
//                         Status.next_state = IDLE;
//                         break;
//                     }
//                     else
//                     {
//                         printf("Halloween_V.mp3 Successfully opened.\n");
//                     }
//                 }

//                 // Read a segment at a time before sending to device
//                 current_segment_size = 0;
//                 result = f_read(&mp3_file, Buffer, MP3_SEGMENT_SIZE, &current_segment_size);
//                 if (result != FR_OK)
//                 {
//                     printf("[MP3Task] mp3 file failed to read. Error: %d\n", result);
//                 }
//                 // Set flag if last segment
//                 if (current_segment_size < MP3_SEGMENT_SIZE)
//                 {
//                     last_segment = true;
//                 }

//                 // Send segment to device
//                 status = MP3Player.PlaySegment((uint8_t*)Buffer, current_segment_size, last_segment);

//                 if (TRANSFER_CANCELLED == status)
//                 {
//                     if (Status.cancel_requested)
//                     {
//                         printf("[MP3Task] Cancel successful.\n");
//                     }
//                     else
//                     {
//                         printf("[MP3Task] Playback cancelled but never requested!\n");
//                     }
//                     // Set last segment flag to run clean up
//                     last_segment = true;
//                     // Reset flags
//                     Status.cancel_requested = false;
//                 }
//                 else if (TRANSFER_FAILED == status)
//                 {
//                     printf("[MP3Task] Segment transfer failed. Stopping playback.\n");
//                     Status.next_state = IDLE;
//                 }

//                 ////////////////////////////
//                 last_segment = true;
//                 ////////////////////////////

//                 // Clean up if last segment
//                 if (last_segment) 
//                 {
//                     result = f_close(&mp3_file);
//                     if (result != FR_OK)
//                     {
//                         printf("[MP3Task] mp3 file failed to close!\n");
//                     }
//                     else
//                     {
//                         printf("[MP3Task] mp3 file closed.\n");
//                     }
//                     Status.next_state = IDLE;
                    
//                 }
//                 break;
//             case STOP:
//                 // If stop requested, is currently playing, and not recently requested
//                 if (MP3Player.IsPlaying() && !Status.cancel_requested)
//                 {
//                     // Stop playback
//                     MP3Player.CancelDecoding();
//                     // Go back to playing to finish the last segment(s)
//                     Status.next_state = PLAY;
//                     // Set the flag for cancel request
//                     Status.cancel_requested = true;
//                 }
//                 // No need to cancel if not currently playing
//                 else
//                 {
//                     Status.next_state = IDLE;
//                 }
//                 break;
//             case FORWARD:
//                 Status.next_state = IDLE;
//                 break;
//             case BACKWARD:
//                 Status.next_state = IDLE;
//                 break;
//         }
//     }

//     void CheckButtons()
//     {
//         // Only one button can be registered at a time
//         if (Button0::getInstance().IsPressed())
//         {
//             Status.next_state = PLAY;
//         }
//         else if (Button1::getInstance().IsPressed())
//         {
//             Status.next_state = STOP;
//         }
//         else if (Button2::getInstance().IsPressed())
//         {
//             // next_state = FORWARD;
//         }
//         else if (Button3::getInstance().IsPressed())
//         {
//             // next_state = BACKWARD;
//         }
//     }

// private:

//     // File pointer to an mp3 file off an SD card
//     FIL          mp3_file;
//     // VS1053b object which handles the device drivers
//     VS1053b      MP3Player;
//     // Buffer for an MP3 segment to send to the device
//     TCHAR        *Buffer;
//     // Status information of the MP3 player
//     MP3_status_S Status;
// };