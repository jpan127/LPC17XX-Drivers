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

void I2C::SetStopFlag()
{
    I2CPtr->I2CONSET |= BIT_I2CONSET_STO;
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

bool I2C::ReadStatus(uint8_t &status)
{
    // If SI is set, read lower byte of status register, else return bogus status
    if (I2CPtr->I2CONSET & BIT_I2CONSET_SI)
    {
        status = (uint8_t)I2CPtr->I2STAT;
        return true;
    }
    else
    {
        status = 0xFF;
        return false;
    }
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
        I2CPtr->I2DAT = data
        return true;
    }
    else
    {
        return false;
    }
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

    // Data
    uint8_t data = 0;

    // Next state
    i2c_slave_state_t next_state = SLAVE_NO_STATE;

    // [TODO] Condition to ACK or NACK
    bool condition = false;

    switch (status)
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        //                                  Receiving states                                     //
        ///////////////////////////////////////////////////////////////////////////////////////////

        case SLAVE_RX_FIRST_ADDRESSED:
        case SLAVE_RX_ARBITRATION_LOST:
        case SLAVE_RX_GENERAL_CALL:
        case SLAVE_RX_ARBITRATION_LOST_GENERAL_CALL:
            SlaveAck();
            ClearSIFlag();
            next_state = (condition) ?  (SLAVE_RX_DATA_RECEIVED_ACK) : 
                                        (SLAVE_RX_DATA_RECEIVED_NACK);
            break;

        case SLAVE_RX_DATA_RECEIVED_ACK:
            data = ReadData();
            ClearSIFlag();
            next_state = (condition) ?  (SLAVE_RX_DATA_RECEIVED_ACK) : 
                                        (SLAVE_RX_DATA_RECEIVED_NACK);
            break;

        case SLAVE_RX_DATA_RECEIVED_NACK:
            data = ReadData();
            ClearSIFlag();
            // If accepting general call SlackAck(), otherwise SlaveNack()
            SlackAck();
            // If generating start condition then SetStartFlag()
            // ???
            // Switching to not addressed mode
            next_state = SLAVE_NO_STATE;
            break;

        case SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_ACK:
            data = ReadData();
            ClearSIFlag();
            next_state = (condition) ?  (SLAVE_RX_DATA_RECEIVED_ACK) : 
                                        (SLAVE_RX_DATA_RECEIVED_NACK);
            break;

        case SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_NACK:
            data = ReadData();
            ClearSIFlag();
            // If accepting general call SlackAck(), otherwise SlaveNack()
            SlackAck();
            // If generating start condition then SetStartFlag()
            // ???
            // Switching to not addressed mode
            next_state = SLAVE_NO_STATE;
            break;

        case SLAVE_RX_STOP_OR_REPEATED_STOP:
            ClearSIFlag();
            // If accepting general call SlackAck(), otherwise SlaveNack()
            SlackAck();
            // If generating start condition then SetStartFlag()
            // ???
            // Switching to not addressed mode
            next_state = SLAVE_NO_STATE;
            break;

        ///////////////////////////////////////////////////////////////////////////////////////////
        //                                  Transmitting states                                  //
        ///////////////////////////////////////////////////////////////////////////////////////////

        case SLAVE_TX_FIRST_ADDRESSED:
        case SLAVE_TX_ARBITRATION_LOST:
        case SLAVE_TX_DATA_TRANSMITTED_ACK:
            ClearSIFlag();
            // Load data
            LoadData(load_data);
            if (last_data)
            {
                SlaveNack();
                next_state = SLAVE_NO_STATE;
            }
            else
            {
                SlaveAck();
                next_state = SLAVE_TX_DATA_TRANSMITTED_ACK;
            }
            break;

        case SLAVE_TX_DATA_TRANSMITTED_NACK:
        case SLAVE_TX_DATA_LAST_TRANSMITTED:
            ClearSIFlag();
            // If accepting general call SlackAck(), otherwise SlaveNack()
            SlackAck();
            // If generating start condition then SetStartFlag()
            // ???
            // Switching to not addressed mode
            next_state = SLAVE_NO_STATE;          
            break;

        case SLAVE_NO_STATE:
            // Nothing
            break;

        case SLAVE_BUS_ERROR:
            ClearSIFlag();
            SetStopFlag();
            printf("[ERROR] I2CSlave::SlaveStateMachine: Bus Error!\n");
            break;

    }

    return state;
}

void I2CSlave::SlaveAck()
{
    SetAAFlag();
}

void I2CSlave::SlackNack()
{
    ClearAAFlag();
}