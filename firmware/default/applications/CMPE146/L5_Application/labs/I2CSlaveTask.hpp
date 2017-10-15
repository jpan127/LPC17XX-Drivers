#pragma once
#include <stdio.h>
#include <scheduler_task.hpp>
#include <task.h>
#include "L5_Application/drivers/i2c.hpp"

class I2CSlaveTask : public scheduler_task
{
public:

    // Constructor
    I2CSlaveTask(uint8_t priority, i2c_port_t port) : scheduler_task("I2CSlaveTask", 8196, priority)
    {
        i2c_slave_addresses_t addresses;
        addresses.address[0] = 0x77;
        addresses.num_addresses = 1;
        I2C1Slave::getInstance().SlaveInitialize(addresses, true);

        I2C1Slave::getInstance().SetDutyCycle(I2C_CLOCK_MODE_400KHZ);
        
        // Load some data into it
        uint32_t buffer_length  = 256;
        uint8_t *buffer         = new uint8_t[buffer_length];
        uint32_t memory_address = 0;

        for (uint32_t i=0; i<buffer_length; i++)
        {
            buffer[i] = i;
        }

        I2C1Slave::getInstance().LoadContiguousDataToMemory(buffer, memory_address, buffer_length);
    }

    bool run(void *p)
    {
        static uint8_t status = 0;

        if (I2C1Slave::getInstance().ReturnState() != status) {
            status = I2C1Slave::getInstance().ReturnState();
            printf("Status: %02X\n", status);
        }

        return true;
    }
};