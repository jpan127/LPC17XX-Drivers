#include "I2CSlaveTask.hpp"

I2CSlaveTask::I2CSlaveTask(i2c_interface_t i2c_interface, 
                            uint16_t stack_size=2048, 
                            uint8_t priority=5) :
							scheduler_task("I2CSlaveTask", stack_size, priority),
							I2C_Base(I2CPtr)
{
	// Save handle to I2C interrupt and register
    // No default case because only 3 enum options
	switch(i2c_interface)
	{
		case I2C0: I2CPtr = LPC_I2C0; IRQPtr = I2C0_IRQn; break;
		case I2C1: I2CPtr = LPC_I2C1; IRQPtr = I2C1_IRQn; break;
		case I2C2: I2CPtr = LPC_I2C2; IRQPtr = I2C2_IRQn; break;
	}
}

void I2CSlaveTask::I2CSlaveInit()
{
	// Enable I2C power/clock control
	switch (IRQPtr)
	{
		case I2C0_IRQn: lpc_pconp(pconp_i2c0, true); break;
		case I2C1_IRQn: lpc_pconp(pconp_i2c1, true); break;
		case I2C2_IRQn: lpc_pconp(pconp_i2c2, true); break;

        default: printf("I2CSlaveTask::I2CSlaveInit: [ERROR] Unhandled IRQPtr option.\n");
	}

    // No need to configure clock as slave

	// Configure slave addresses
    I2CPtr->I2ADR0 = 0;
    I2CPtr->I2ADR1 = 1;
    I2CPtr->I2ADR2 = 2;
    I2CPtr->I2ADR3 = 3;

    // Configure address mask
    I2CPtr->I2MASK0 = 0;
    I2CPtr->I2MASK1 = 0;
    I2CPtr->I2MASK2 = 0;
    I2CPtr->I2MASK3 = 0;

    // Clear all control bits [X110 11XX] except for STO (stop)
    I2CPtr->I2CONCLR = 0x6C;

    // Enable I2C operation
    I2CPtr->I2CONSET = SLAVE_CONTROL_START;

    // Enable I2C interrupt
    NVIC_EnableIRQ(IRQPtr);
}

I2CSlaveTask::i2c_slave_status_t I2CSlaveTask::I2CSlaveStateMachine()
{
    // General call is for broadcasting to all devices

    // Slave states, no instantiation
    enum
    {
        // Receiver
        RX_ADDRESS_ACK                          = 0x60, // Master sends START 0x08
        RX_ARBITRATION_LOST_ADDRESS_ACK         = 0x68, // 
        RX_GENERAL_CALL_ACK                     = 0x70,
        RX_ARBITRATION_LOST_GENERAL_CALL_ACK    = 0x78,
        RX_DATA_ACK                             = 0x80,
        RX_DATA_NACK                            = 0x88,
        RX_GENERAL_CALL_DATA_ACK                = 0x90,
        RX_GENERAL_CALL_DATA_NACK               = 0x98,
        RX_STOP_OR_REPEAT_START                 = 0xA0,
        // Transmitter
        TX_ADDRESS_ACK                          = 0xA8,
        TX_ARBITRATION_LOST_ACK                 = 0xB0,
        TX_DATA_SENT_ACK                        = 0xB8,
        TX_DATA_SENT_NACK                       = 0xC0,
        TX_LAST_DATA_SENT_ACK                   = 0xC8,
        // Miscellaneous
        NO_STATE                                = 0xF8,
        BUS_ERROR                               = 0x00
    };

    // Current status, return value
    i2c_slave_status_t status = BUSY;

    switch (I2CPtr->I2STAT)
    {
        case RX_ADDRESS_ACK:
            //
            break;
        case RX_ARBITRATION_LOST_ADDRESS_ACK:
            //
            break;
        case RX_GENERAL_CALL_ACK:
            //
            break;
        case RX_ARBITRATION_LOST_GENERAL_CALL_ACK:
            //
            break;
        case RX_DATA_ACK:
            //
            break;
        case RX_DATA_NACK:
            //
            break;
        case RX_GENERAL_CALL_DATA_ACK:
            //
            break;
        case RX_GENERAL_CALL_DATA_NACK:
            //
            break;
        case RX_STOP_OR_REPEAT_START:
            //
            break;
        case TX_ADDRESS_ACK:
            //
            break;
        case TX_ARBITRATION_LOST_ACK:
            //
            break;
        case TX_DATA_SENT_ACK:
            //
            break;
        case TX_DATA_SENT_NACK:
            //
            break;
        case TX_LAST_DATA_SENT_ACK:
            //
            break;
        case NO_STATE:
            //
            break;
        case BUS_ERROR:
            //
            break;
        default:
            // Don't use printf because this will be called from ISR!!
            printf("I2CSlaveTask::I2CSlaveStateMachine: [ERROR] Unhandled I2C status code: %lu.\n", 
                                                                                I2CPtr->I2STAT);
    }

    return status;
}

inline void I2CSlaveTask::ClearSIFlag()
{
    I2CPtr->I2CONCLR |= BIT_SI;
}

inline void I2CSlaveTask::SetStartFlag()
{
    I2CPtr->I2CONSET |= BIT_STA;
}

inline void I2CSlaveTask::ClearStartFlag()
{
    I2CPtr->I2CONSET |= BIT_STA;
}

inline void I2CSlaveTask::SetAAFlag()
{
    I2CPtr->I2CONSET |= BIT_AA;
}

inline void I2CSlaveTask::ClearAAFlag()
{
    I2CPtr->I2CONCLR |= BIT_AA;
}

inline uint8_t I2CSlaveTask::ReadStatus()
{
    // Check if SI is set first
    if (I2CPtr->I2CONSET & BIT_SI) {
        // Read lower byte of status register
        return (uint8_t)I2CPtr->I2STAT;
    }
    else {
        // Return bogus status
        return (uint8_t)0xFF;
    }
}