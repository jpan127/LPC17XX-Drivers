#include "i2c.hpp"
#include <uart0_min.h>

extern "C"
{
    void I2C1_IRQHandler()
    {
        I2C1Slave::getInstance().SlaveStateMachine();
    }

    // // Used by all the onboard sensors
    // void I2C2_IRQHandler()
    // {
    //     I2C2Slave::getInstance().SlaveStateMachine();
    // }
}

I2C::I2C(i2c_port_t port)
{
    Port = port;

    switch (Port)
    {
        case I2C_PORT0: I2CPtr = LPC_I2C0; IRQPtr = I2C0_IRQn; break;
        case I2C_PORT1: I2CPtr = LPC_I2C1; IRQPtr = I2C1_IRQn; break;
        case I2C_PORT2: I2CPtr = LPC_I2C2; IRQPtr = I2C2_IRQn; break;
    }
}

void I2C::Initialize()
{
    // Configure I2C pins
    switch (Port)
    {
        case I2C_PORT0:
            // Turn on peripheral clock
            LPC_SC->PCONP           |=  (1 << 7);
            // Select I2C pin function
            LPC_PINCON->PINSEL1     &= ~(0xF << 22);    // SDA0 : P0.27
            LPC_PINCON->PINSEL1     |=  (0x5 << 22);    // SCL0 : P0.28
            // Disable pull up / pull down resistors
            // I2C_PORT0 is automatically set for open drain

            // Can configure extra settings only for I2C_PORT0
            // Fast mode plus drive mode, glitch filtering and slew rate control
            break;

        case I2C_PORT1:
            // Turn on peripheral clock
            LPC_SC->PCONP           |=  (1 << 19);
            // Select I2C pin function
            LPC_PINCON->PINSEL0     &= ~(0xF << 0);     // SDA0 : P0.0
            LPC_PINCON->PINSEL0     |=  (0xF << 0);     // SCL0 : P0.1
            // Disable pull up / pull down resistors
            LPC_PINCON->PINMODE0    &= ~(0xF << 0);
            LPC_PINCON->PINMODE0    |=  (0xA << 0);
            // Turn on open drain
            LPC_PINCON->PINMODE_OD0 |=  (0x3 << 0);
            break;

        case I2C_PORT2:
            // Turn on peripheral clock
            LPC_SC->PCONP           |=  (1 << 26);
            // Select I2C pin function
            LPC_PINCON->PINSEL0     &= ~(0xF << 20);     // SDA0 : P0.10 (0.19 too)
            LPC_PINCON->PINSEL0     |=  (0xA << 20);     // SCL0 : P0.11 (0.20 too)
            // Disable pull up / pull down resistors
            LPC_PINCON->PINMODE0    &= ~(0xF << 20);
            LPC_PINCON->PINMODE0    |=  (0xA << 20);
            // Turn on open drain
            LPC_PINCON->PINMODE_OD0 |=  (0x3 << 10);
            break;
    }

    // Enable interrupts
    NVIC_EnableIRQ(IRQPtr);
}

void I2C::SetDutyCycle(i2c_clock_mode_t mode)
{
    uint32_t baud      = 0;
    uint32_t pclk      = sys_get_cpu_clock();
    uint32_t duty      = 0;
    uint16_t high_duty = 0;
    uint16_t low_duty  = 0;

    // duty = pclk / (low_duty + high_duty)
    // (low_duty + high_duty) = pclk / duty
        
    switch (mode)
    {
        case I2C_CLOCK_MODE_100KHZ:
            baud       = 100000;
            duty       = pclk / 100000;
            high_duty  = duty / 2;
            low_duty   = duty / 2;
            break;

        case I2C_CLOCK_MODE_400KHZ:
            baud       = 400000;
            duty       = pclk / 400000;
            high_duty  = duty / 2;
            low_duty   = duty / 2;
            break;

        case I2C_CLOCK_MODE_1MHZ:
            baud       = 1000000;
            duty       = pclk / 1000000;
            high_duty  = duty / 2;
            low_duty   = duty / 2;
            break;
    }

    printf("[I2C] Baud: %lu | Pclk: %lu | Duty: %lu | High: %u | Low: %u\n", baud, pclk, duty, 
                                                                          high_duty, low_duty);

    I2CPtr->I2SCLH = high_duty;
    I2CPtr->I2SCLL = low_duty;
}

void I2C::ClearSIFlag()
{
    I2CPtr->I2CONCLR = BIT_I2CONSET_SI;
}

void I2C::ClearStartFlag()
{
    I2CPtr->I2CONCLR = BIT_I2CONSET_STA;
}

void I2C::ClearAAFlag()
{
    I2CPtr->I2CONCLR = BIT_I2CONSET_AA;
}

void I2C::SetStartFlag()
{
    I2CPtr->I2CONSET = BIT_I2CONSET_STA;
}

void I2C::SetStopFlag()
{
    I2CPtr->I2CONSET = BIT_I2CONSET_STO;
}

void I2C::SetAAFlag()
{
    I2CPtr->I2CONSET = BIT_I2CONSET_AA;
}

uint8_t I2C::ReadStatus()
{
    return (uint8_t)I2CPtr->I2STAT;
}

bool I2C::ReadData(uint8_t &data)
{
    if (I2CPtr->I2CONSET & BIT_I2CONSET_SI)
    {
        data = (uint8_t)I2CPtr->I2DAT;
        return true;
    }
    else
    {
        data = 0xFF;
        return false;
    }
}

bool I2C::LoadData(uint8_t data)
{
    if (I2CPtr->I2CONSET & BIT_I2CONSET_SI)
    {
        I2CPtr->I2DAT = data;
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void I2CSlave::SlaveAck()
{
    SetAAFlag();
}

void I2CSlave::SlaveNack()
{
    ClearAAFlag();
}

I2CSlave::I2CSlave(i2c_port_t port) : I2C(port)
{
    // Set slave to no state
    SlaveCurrentState = SLAVE_NO_STATE;
    // Initialize TX / RX buffers
    SlaveTxBuffer.buffer        = new uint8_t[SLAVE_TX_BUFFER_SIZE]();
    SlaveRxBuffer.buffer        = new uint8_t[SLAVE_RX_BUFFER_SIZE]();
    SlaveTxBuffer.buffer_index  = 0;
    SlaveRxBuffer.buffer_index  = 0;
    SlaveTxBuffer.buffer_size   = SLAVE_TX_BUFFER_SIZE;
    SlaveRxBuffer.buffer_size   = SLAVE_RX_BUFFER_SIZE;
    // Allocate memory
    Memory = new buffer_t;
    Memory->buffer          = new uint8_t[SLAVE_MEMORY_SIZE]();
    Memory->buffer_index    = 0;
    Memory->buffer_size     = SLAVE_MEMORY_SIZE;
    MemoryNeedsDestruction  = true;
}

I2CSlave::I2CSlave(i2c_port_t port, buffer_t *memory_buffer) : I2C(port)
{
    // Set slave to no state
    SlaveCurrentState = SLAVE_NO_STATE;
    // Initialize TX / RX buffers
    SlaveTxBuffer.buffer        = new uint8_t[SLAVE_TX_BUFFER_SIZE]();
    SlaveRxBuffer.buffer        = new uint8_t[SLAVE_RX_BUFFER_SIZE]();
    SlaveTxBuffer.buffer_index  = 0;
    SlaveRxBuffer.buffer_index  = 0;
    SlaveTxBuffer.buffer_size   = SLAVE_TX_BUFFER_SIZE;
    SlaveRxBuffer.buffer_size   = SLAVE_RX_BUFFER_SIZE;
    // Point to memory_buffer
    Memory = memory_buffer;
    MemoryNeedsDestruction = false;
}

I2CSlave::~I2CSlave()
{
    delete [] SlaveTxBuffer.buffer;
    delete [] SlaveRxBuffer.buffer;

    if (MemoryNeedsDestruction) 
    {
        delete [] Memory->buffer;
        Memory->buffer = NULL;
        delete Memory;
        Memory = NULL;
    }
}

void I2CSlave::LoadContiguousDataToMemory(  uint8_t *buffer, 
                                            uint32_t memory_address, 
                                            uint32_t buffer_length)
{
    // Only checks to see if input goes out of bounds
    // Does not check to see if input will overwrite
    if (Memory->buffer_size < (memory_address + buffer_length))
    {
        printf("[ERROR] I2CSlave::LoadContiguousDataToMemory buffer larger than memory!\n");
    }
    else
    {
        for (uint32_t i=0; i<buffer_length; i++)
        {
            LoadByteToMemory(buffer[i], memory_address+i);
        }
    }
}

void I2CSlave::LoadByteToMemory(uint8_t data, uint32_t memory_address)
{
    Memory->buffer[memory_address] = data;
    Memory->buffer_index++;
}

uint32_t I2CSlave::ReadContiguousDataFromMemory(uint8_t *buffer, 
                                                uint32_t memory_address, 
                                                uint32_t buffer_length)
{
    uint32_t data_read = 0;

    // Checks to see if reading won't read out of bounds
    if (Memory->buffer_size < memory_address + buffer_length)
    {
        printf("[ERROR] I2CSlave::ReadContiguousDataFromMemory buffer larger than memory!\n");
    }
    else
    {
        for (uint32_t i=0; i<buffer_length; i++)
        {
            buffer[i] = ReadByteFromMemory(memory_address + i);
            data_read++;
        }
    }

    return data_read;
}

uint8_t I2CSlave::ReadByteFromMemory(uint32_t memory_address)
{
    return Memory->buffer[memory_address];
}

void I2CSlave::SlaveInitialize(i2c_slave_addresses_t addresses, bool enable_general_call)
{
    // Call base class initialize
    Initialize();

    // Set address registers
    // For number of addresses we are intializing, set the corresponding register
    for (int i=0; i<addresses.num_addresses; i++)
    {
        // Change address to 7 bits and set the general call bit
        uint8_t addr = (addresses.address[i] << 1) | enable_general_call;

        // Set register
        switch (i)
        {
            case 0: I2CPtr->I2ADR0 = addr; break;
            case 1: I2CPtr->I2ADR1 = addr; break;
            case 2: I2CPtr->I2ADR2 = addr; break;
            case 3: I2CPtr->I2ADR3 = addr; break;
        }
    }

    I2CPtr->I2MASK0 = 0;
    I2CPtr->I2MASK1 = 0;
    I2CPtr->I2MASK2 = 0;
    I2CPtr->I2MASK3 = 0;

    // Initialize I2CON register with only AA and I2EN
    I2CPtr->I2CONSET = I2CONSET_AA_I2EN;
}

i2c_state_status_t I2CSlave::SlaveStateMachine()
{
    // Static variables
    static  uint32_t memory_address  = 0;
    static  bool     first_byte      = true;

    // Non-static variables
    uint8_t status  = 0;                // Read status register
    uint8_t rx_data = 0;                // Data
    i2c_state_status_t state = IDLE;    // State machine's status

    // Condition to ACK or NACK
    // Check to see if memory address to register is out of bounds
    const bool memory_overrun = (memory_address >= Memory->buffer_size);

    status = ReadStatus();
    switch (status)
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        //                                  Receiving states                                     //
        ///////////////////////////////////////////////////////////////////////////////////////////

        case SLAVE_RX_FIRST_ADDRESSED:                                      // 0x60
        case SLAVE_RX_GENERAL_CALL:                                         // 0x70
        case SLAVE_RX_ARBITRATION_LOST:                                     // 0x68
        case SLAVE_RX_ARBITRATION_LOST_GENERAL_CALL:                        // 0x78
            SlaveAck();
            ClearSIFlag();
            break;

        case SLAVE_RX_DATA_RECEIVED_ACK:                                    // 0x80
            ReadData(rx_data);
            ClearSIFlag();
            // First byte holds register/memory address
            if (first_byte)
            {
                memory_address = rx_data;
                first_byte     = false;
                state          = BUSY;
            }
            // If memory overrun just clear AA bit
            else if (memory_overrun)
            {
                SlaveNack();
            }
            // Not first byte, load byte into memory, increment address
            else
            {
                Memory->buffer[memory_address] = rx_data;
                Memory->buffer_index++;
                memory_address++;
                SlaveAck();
                state = BUSY;
            }
            break;

        case SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_ACK:                       // 0x90
            // Read general call's first byte, not sure what to do with it
            first_byte = true;
            ReadData(rx_data);
            ClearSIFlag();
            ClearAAFlag();
            break;

        case SLAVE_RX_DATA_RECEIVED_NACK:                                   // 0x88
        case SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_NACK:                      // 0x98
        case SLAVE_RX_STOP_OR_REPEATED_STOP:                                // 0xA0
            SlaveAck();
            ClearStartFlag();
            ClearSIFlag();
            // Reset so next first byte will store register address
            first_byte = true;
            break;

        ///////////////////////////////////////////////////////////////////////////////////////////
        //                                  Transmitting states                                  //
        ///////////////////////////////////////////////////////////////////////////////////////////

        case SLAVE_TX_FIRST_ADDRESSED:                                      // 0xA8
        case SLAVE_TX_ARBITRATION_LOST:                                     // 0xB0
        case SLAVE_TX_DATA_TRANSMITTED_ACK:                                 // 0xB8
            if (memory_overrun)
            {
                ClearAAFlag();
            }
            else
            {
                LoadData(Memory->buffer[memory_address]);
                memory_address++;
                Memory->buffer_index++;
                SetAAFlag();
                state = BUSY;
            }
            ClearSIFlag();
            break;

        case SLAVE_TX_DATA_TRANSMITTED_NACK:                                // 0xC0
        case SLAVE_TX_DATA_LAST_TRANSMITTED:                                // 0xC8
            SlaveAck();
            ClearSIFlag();
            break;

        case SLAVE_NO_STATE:                                                // 0xF8
            break;

        case SLAVE_BUS_ERROR:                                               // 0x00
            ClearSIFlag();
            SetStopFlag();
            ClearStartFlag();
            break;
    }

    return state;
}

uint8_t I2CSlave::ReturnState()
{
    return ReadStatus();
}