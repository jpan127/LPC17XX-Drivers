#pragma once
#include <stdio.h>
#include <string.h>
#include <scheduler_task.hpp>
#include <task.h>
#include <i2c2.hpp>
#include "L5_Application/drivers/buttons.hpp"

static const char I2C_MSG[] = "HICMPE146!";
//{ 0x76, 0x30, 0x39, 0x00, 0x73, 0x79, 0x30, 0x66, 0x7C, 0x86 };
const int I2C_MSG_LEN = strlen(I2C_MSG);

class I2CMasterTask : public scheduler_task
{
public:

    // Constructor
    I2CMasterTask(uint8_t priority) : scheduler_task("I2CMasterTask", 8196, priority)
    {  
        // Initialize with 400k baud rate
        I2C2::getInstance().init(400000);

        buffer = new uint8_t[1024]();
    }

    bool run(void *p)
    {
        static int msg_index = 0;


        // Button 0 reads first 128 bytes from slave
        if (Button0::getInstance().IsPressed()) {
            printf("[I2CMasterTask] Button 0 pressed, reading from slave...\n");
            // Make sure completely clear
            memset(buffer, 0, 1024);
            I2C2::getInstance().readRegisters(0xEE, 0, buffer, 128);
            for (int i=0; i<128; i++) {
                printf("%i ", buffer[i]);
            }
            puts("");
        }

        // Button 1 writes to the first 128 bytes of slave
        if (Button1::getInstance().IsPressed()) {
            printf("[I2CMasterTask] Button 1 pressed, doubling and writing to slave...\n");
            for (int i=0; i<128; i++) {
                buffer[i] *= 2;
            }
            I2C2::getInstance().writeRegisters(0xEE, 0, buffer, 128);
        }

        // Button 2 increments everything
        if (Button2::getInstance().IsPressed()) {
            printf("[I2CMasterTask] Button 2 pressed, incrementing and writing to slave...\n");
            for (int i=0; i<128; i++) {
                buffer[i] += 1;
            }
            I2C2::getInstance().writeRegisters(0xEE, 0, buffer, 128);
        }

        // Button 3 makes the slave say HI
        if (Button3::getInstance().IsPressed()) {
            printf("[I2CMasterTask] Button 3 pressed, writing to slave...\n");
            // Write message
            for (int i=0; i<I2C_MSG_LEN; i++) {
                memset(buffer, 0, 1);
                buffer[0] = I2C_MSG[i];
                printf("Writing: %c\n", (char)buffer[0]);
                I2C2::getInstance().writeRegisters(0xEE, 200, buffer, 1);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }

        return true;
    }

    uint8_t *buffer;

};