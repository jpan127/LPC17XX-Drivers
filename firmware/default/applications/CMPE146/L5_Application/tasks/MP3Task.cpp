#include "MP3Task.hpp"


static const vs1053b_gpio_init_t gpio_init = {
    .port_reset = GPIO_PORT0,
    .port_dreq  = GPIO_PORT0,
    .port_xcs   = GPIO_PORT0,
    .port_xdcs  = GPIO_PORT0,
    .pin_reset  = 0,
    .pin_dreq   = 1,
    .pin_xcs    = 29,
    .pin_xdcs   = 30,
};

// File pointer to an mp3 file off an SD card
static FIL          mp3_file;
// VS1053b object which handles the device drivers
static VS1053b      MP3Player(gpio_init);
// Buffer for an MP3 segment to send to the device
static TCHAR        Buffer[MP3_SEGMENT_SIZE];
// Status information of the MP3 player
static MP3_status_S Status;

const char name[] = "1:track3.mp3";

static void HandleStateLogic()
{
    static uint32_t segment_counter  = 0;
    static UINT current_segment_size = 0;
    static bool last_segment 		 = false;
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
                result = f_open(&mp3_file, name, FA_OPEN_EXISTING | FA_READ);
                if (result != FR_OK)
                {
                    printf("[MP3Task] mp3 file failed to open. Error: %d\n", result);
                    Status.next_state = IDLE;
                    break;
                }
                else
                {
                    printf("%s successfully opened.\n", name);
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

            // printf("[MP3Task] Played segment %lu with %d bytes.\n", segment_counter++, current_segment_size);

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
    // Initialize status struct
    Status.cancel_requested = false;
    Status.next_state = IDLE;

    memset(Buffer, 0, sizeof(Buffer));
    ssp0_init(0);
    MP3Player.SystemInit();

    while (1)
    {
        CheckButtons();
        HandleStateLogic();
    }
}