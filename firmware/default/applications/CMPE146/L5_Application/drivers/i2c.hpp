#include <stdio.h>
#include <sys_config.h>

#define PACKED (__attribute__((packed)))

// I2CONSET bits
#define BIT_I2CONSET_AA     (1 << 2)    // Assert Acknowledge Flag
#define BIT_I2CONSET_SI     (1 << 3)    // I2C Interrupt Flag
#define BIT_I2CONSET_STO    (1 << 4)    // Stop Flag
#define BIT_I2CONSET_STA    (1 << 5)    // Start Flag
#define BIT_I2CONSET_I2EN   (1 << 6)    // I2C Interface Enable Flag

// States
#define SLAVE_CONTROL_START (0x44)      // [X100 01XX]

// Enums
typedef enum 
{ 
    I2C_PORT0, 
    I2C_PORT1, 
    I2C_PORT2 
} PACKED i2c_port_t;

typedef enum 
{ 
    I2C_CLOCK_MODE_100KHZ,      // Standard
    I2C_CLOCK_MODE_400KHZ,      // Fast
    I2C_CLOCK_MODE_1MHZ         // Fast Mode Plus
} PACKED i2c_clock_mode_t;

// Maybe not necessary as there are internal states
typedef enum 
{
    I2C_MASTER_TX,              // Master transmitting
    I2C_MASTER_RX,              // Master receiving
    I2C_MASTER_IDLE,            // Master not doing anything
    I2C_SLAVE_TX,               // Slave transmitting
    I2C_SLAVE_RX,               // Slave reading
    I2C_SLAVE_IDLE              // Slave not doing anything
} PACKED i2c_mode_t;

class I2C
{
protected:

    // Constructor
    I2C(i2c_port_t port);

    // Initialize I2C interface
    void Initialize(uint32_t duty, i2c_clock_mode_t mode);

    // Member Variables
    i2c_port_t      Port;
    LPC_I2C_TypeDef *I2CPtr
    IRQn_Type       IRQPtr
};

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{

} PACKED i2c_master_state_t;

class I2CMaster : public I2C
{
public:

    // Constructor
    I2CMaster(i2c_port_t port);

    /*
        @description            :    Master writes to a slave
        @param slave_address    :    Address of slave being targeted
        @param buffer           :    Pre-allocated buffer of data being sent
        @param buffer_length    :    Length of buffer to be sent
        @return                 :    Current state of master
    */
    i2c_master_state_t MasterWrite( uint8_t     slave_address,
                                    uint8_t     *buffer,
                                    uint32_t    buffer_length);

    /*
        @description            :    Master reads from a slave
        @param slave_address    :    Address of slave being targeted
        @param buffer           :    Pre-allocated buffer to be filled with data
        @param buffer_length    :    Length of buffer sent
        @return                 :    Current state of master
    */
    i2c_master_state_t MasterRead(  uint8_t     slave_address,
                                    uint8_t     *buffer,
                                    uint32_t    &buffer_length);

    
}