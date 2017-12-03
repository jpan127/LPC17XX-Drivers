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

void MP3Task(void *p);