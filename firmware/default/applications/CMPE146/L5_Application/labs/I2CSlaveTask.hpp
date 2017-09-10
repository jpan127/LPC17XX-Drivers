#include <scheduler_task.hpp>
#include <task.h>
#include <stdio.h>
#include <i2c_base.hpp>

#define SLAVE_CONTROL_START (0x44)      // [X100 01XX]
#define BIT_AA              (1 << 2)
#define BIT_SI              (1 << 3)
#define BIT_STO             (1 << 4)
#define BIT_STA             (1 << 5)
#define BIT_I2EN            (1 << 6)

typedef enum { I2C0, I2C1, I2C2 } i2c_interface_t;

/*
    Write Transaction:
        Master sends START
        Master sends ADDRESS + W
        Master sends REGISTER to start reading from
        Master sends REPEAT START
        Master sends ADDRESS + R
        Master reads byte, ACKS if not last byte, NACK if last byte
        Master sends STOP
    Read Transaction:

*/

class I2CSlaveTask : public scheduler_task, public I2C_Base
{
public:

    // Constructor
    // Use I2C0, I2C1, or I2C2
    I2CSlaveTask(i2c_interface_t i2c_interface, uint16_t stack_size, uint8_t priority);

    // Initialize
    void                I2CSlaveInit();

private:

    typedef enum {
        BUSY, 
        WRITE_COMPLETE,
        READ_COMPLETE
    } __attribute__((packed)) i2c_slave_status_t;

    // State Machine
    i2c_slave_status_t  I2CSlaveStateMachine();

    inline void         ClearSIFlag();

    inline void         SetStartFlag();

    inline void         ClearStartFlag();

    // In SR mode, when the AA is reset the slave will not respond until it is set again
    inline void         SetAAFlag();

    inline void         ClearAAFlag();

    inline uint8_t      ReadStatus();
    
    LPC_I2C_TypeDef    *I2CPtr;    // Pointer to I2C register
    
    IRQn_Type           IRQPtr;    // Pointer to I2c interrupt
};