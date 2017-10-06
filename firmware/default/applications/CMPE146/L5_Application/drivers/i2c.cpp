#include "i2c.hpp"

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
            // Turn in peripheral clock
            LPC_SC->PCONP       |=  (0x3 << 14);
            // Select I2C pin function
            LPC_PINCON->PINSEL1 &= ~(0xF << 22);    // SDA0 : P0.27
            LPC_PINCON->PINSEL1 |=  (0x5 << 22);    // SCL0 : P0.28
            // Disable pull up / pull down resistors
            // I2C_PORT0 is automatically set for open drain

            // Can configure extra settings only for I2C_PORT0
            // Fast mode plus drive mode, glitch filtering and slew rate control
            break;

        case I2C_PORT1:
            // Turn in peripheral clock
            LPC_SC->PCONP       |=  (0x3 << 6);
            // Select I2C pin function
            LPC_PINCON->PINSEL0 &= ~(0xF << 0);     // SDA0 : P0.0
            LPC_PINCON->PINSEL0 |=  (0xF << 0);     // SCL0 : P0.1
            // Disable pull up / pull down resistors
            LPC_PINCON->PINMODE &= ~(0xF << 0);
            LPC_PINCON->PINMODE |=  (0xA << 0);
            // Turn on open drain
            LPC_PINCON->PINMODE_OD0 |= (0x3 << 0);
            break;

        case I2C_PORT2:
            // Turn in peripheral clock
            LPC_SC->PCONP       |=  (0x3 << 20);
            // Select I2C pin function
            LPC_PINCON->PINSEL0 &= ~(0xF << 20);     // SDA0 : P0.10 (0.19 too)
            LPC_PINCON->PINSEL0 |=  (0xA << 20);     // SCL0 : P0.11 (0.20 too)
            // Disable pull up / pull down resistors
            LPC_PINCON->PINMODE &= ~(0xF << 20);
            LPC_PINCON->PINMODE |=  (0xA << 20);
            // Turn on open drain
            LPC_PINCON->PINMODE_OD0 |= (0x3 << 10);
            break;
    }

    // Enable interrupts
    NVIC_EnableIRQ(IRQPtr);
}

void I2C::SetDutyCycle(uint32_t duty, i2c_clock_mode_t mode)
{
    uint32_t pclk      = sys_get_cpu_clock();
    uint32_t duty      = 0;
    uint16_t high_duty = 0;
    uint16_t low_duty  = 0;

    // duty = pclk / (low_duty + high_duty)
    // (low_duty + high_duty) = pclk / duty
        
    switch (mode)
    {
        case I2C_CLOCK_MODE_100KHZ:
            duty       = pclk / 100000;
            high_duty  = duty / 2;
            low_duty   = duty / 2;
            break;

        case I2C_CLOCK_MODE_400KHZ:
            duty       = pclk / 400000;
            high_duty  = duty / 2;
            low_duty   = duty / 2;
            break;

        case I2C_CLOCK_MODE_1MHZ:
            duty       = pclk / 1000000;
            high_duty  = duty / 2;
            low_duty   = duty / 2;
            break;
    }

    I2CPtr->I2SCLH = high_duty;
    I2CPtr->I2SCLL = low_duty;
}

void I2C::ClearSIFlag()
{
    I2CPtr->I2CONCLR |= BIT_I2CONSET_SI;
}

void I2C::SetStartFlag()
{
    I2CPtr->I2CONSET |= BIT_I2CONSET_STA;
}

void I2C::ClearStartFlag()
{
    I2CPtr->I2CONCLR |= BIT_I2CONSET_STA;
}

void I2C::SetAAFlag()
{
    I2CPtr->I2CONSET |= BIT_I2CONSET_AA;
}

void I2C::ClearAAFlag()
{
   I2CPtr->I2CONCLR |= BIT_I2CONSET_AA;
}

uint8_t I2C::ReadStatus()
{
    // If SI is set, read lower byte of status register, else return bogus status
    return (I2CPtr->I2CONSET & BIT_I2CONSET_SI) ? ((uint8_t)I2CPtr->I2STAT) : 0xFF;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

I2CSlave::I2CSlave(i2c_port_t port) : I2C(port)
{
    SlaveCurrentState   = SLAVE_NO_STATE;
    SlaveTxBuffer       = new char[SLAVE_TX_BUFFER_SIZE];
    SlaveRxBuffer       = new char[SLAVE_RX_BUFFER_SIZE];
}

I2CSlave::~I2CSlave()
{
    delete [] SlaveTxBuffer;
    delete [] SlaveRxBuffer;
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

    // Initialize I2CON register with only AA and I2EN
    I2CPtr->I2CONSET = I2CONSET_AA_I2EN;
}

i2c_slave_state_t I2CSlave::SlaveStateMachine()
{
    // Read status register
    uint8_t status = ReadStatus();

    // Next state

    switch (status)
    {
        case SLAVE_RX_FIRST_ADDRESSED:
            SET_BIT(I2CPtr->I2CONSET, BIT_I2CONSET_AA);
            CLEAR_BIT(I2CPtr->I2CONSET, BIT_I2CONSET_SI);
            break;
        case SLAVE_RX_ARBITRATION_LOST:
            
            break;
        case SLAVE_RX_GENERAL_CALL:
            
            break;
        case SLAVE_RX_ARBITRATION_LOST_GENERAL_CALL:
            
            break;
        case SLAVE_RX_DATA_RECEIVED_ACK:
            
            break;
        case SLAVE_RX_DATA_RECEIVED_NACK:
            
            break;
        case SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_ACK:
            
            break;
        case SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_NACK:
            
            break;
        case SLAVE_RX_STOP_OR_REPEATED_STOP:
            
            break;
        case SLAVE_TX_FIRST_ADDRESSED:
            
            break;
        case SLAVE_TX_ARBITRATION_LOST:
            
            break;
        case SLAVE_TX_DATA_TRANSMITTED_ACK:
            
            break;
        case SLAVE_TX_DATA_TRANSMITTED_NACK:
            
            break;
        case SLAVE_TX_DATA_LAST_TRANSMITTED:
            
            break;
        case SLAVE_NO_STATE:
            
            break;
        case SLAVE_BUS_ERROR:
            
            break;
    }
}