#pragma once
#include <stdio.h>
#include <scheduler_task.hpp>
#include <task.h>
#include <i2c2.hpp>
#include "L5_Application/drivers/buttons.hpp"

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
        // Button 0 reads first 256 bytes from slave
        if (Button0::getInstance().IsPressed()) {
            printf("[I2CMasterTask] Button 0 pressed, reading from slave...\n");
            I2C2::getInstance().readRegisters(0xEE, 0, buffer, 256);
            for (int i=0; i<256; i++) {
                printf("%i ", buffer[i]);
            }
            puts("");
        }

        // Button 1 writes to the first 256 bytes of slave
        if (Button1::getInstance().IsPressed()) {
            printf("[I2CMasterTask] Button 1 pressed, doubling and writing to slave...\n");
            for (int i=0; i<256; i++) {
                buffer[i] *= 2;
            }
            I2C2::getInstance().writeRegisters(0xEE, 0, buffer, 256);
        }

        // Button 2 increments everything
        if (Button2::getInstance().IsPressed()) {
            printf("[I2CMasterTask] Button 2 pressed, incrementing and writing to slave...\n");
            for (int i=0; i<256; i++) {
                buffer[i] += 1;
            }
            I2C2::getInstance().writeRegisters(0xEE, 0, buffer, 256);
        }

        return true;
    }

    uint8_t *buffer;

};