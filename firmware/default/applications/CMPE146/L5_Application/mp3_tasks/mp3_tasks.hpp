#pragma once
#include <queue.h>

// Queues
QueueHandle_t MessageRxQueue = xQueueCreate(3, sizeof(command_packet_S));
QueueHandle_t MessageTxQueue = xQueueCreate(3, sizeof(diagnostic_packet_S));

// Task for communicating with the VS1053b device
void MP3Task(void *p);

// Task for communicating over UART with the ESP32
void ESP32Task(void *p);