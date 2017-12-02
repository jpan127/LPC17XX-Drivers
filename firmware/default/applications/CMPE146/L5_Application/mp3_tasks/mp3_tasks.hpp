#pragma once
#include <scheduler_task.hpp>


// Task for communicating with the VS1053b device
void MP3Task(void *p);

// Task for communicating over UART with the ESP32
void ESP32Task(void *p);