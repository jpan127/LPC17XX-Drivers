#include <stdio.h>
#include <sys_config.h>
#include <LPC17xx.h>
#include <singleton_template.hpp>

// Helper macros
#define CLEAR_BIT(var, bit) (var &= ~(bit))
#define SET_BIT(var, bit)   (var |=  (bit))

// Packed attribute for enums and structs
#define PACKED              __attribute__((__packed__))

// I2CONSET bits
#define BIT_I2CONSET_AA     (1 << 2)    // Assert Acknowledge Flag
#define BIT_I2CONSET_SI     (1 << 3)    // I2C Interrupt Flag
#define BIT_I2CONSET_STO    (1 << 4)    // Stop Flag
#define BIT_I2CONSET_STA    (1 << 5)    // Start Flag
#define BIT_I2CONSET_I2EN   (1 << 6)    // I2C Interface Enable Flag
#define I2CONSET_AA_I2EN    (BIT_I2CONSET_AA | BIT_I2CONSET_I2EN)

// I2C ports
typedef enum 
{ 
    I2C_PORT0, 
    I2C_PORT1, 
    I2C_PORT2 
} 
PACKED i2c_port_t;

// I2C clock speed modes
typedef enum 
{ 
    I2C_CLOCK_MODE_100KHZ,      // Standard
    I2C_CLOCK_MODE_400KHZ,      // Fast
    I2C_CLOCK_MODE_1MHZ         // Fast Mode Plus
} 
PACKED i2c_clock_mode_t;

/*
    How the I2C register structure looks like:
        typedef struct
        {
            __IO uint32_t I2CONSET;
            __I  uint32_t I2STAT;
            __IO uint32_t I2DAT;
            __IO uint32_t I2ADR0;
            __IO uint32_t I2SCLH;
            __IO uint32_t I2SCLL;
            __O  uint32_t I2CONCLR;
            __IO uint32_t MMCTRL;
            __IO uint32_t I2ADR1;
            __IO uint32_t I2ADR2;
            __IO uint32_t I2ADR3;
            __I  uint32_t I2DATA_BUFFER;
            __IO uint32_t I2MASK0;
            __IO uint32_t I2MASK1;
            __IO uint32_t I2MASK2;
            __IO uint32_t I2MASK3;
        } LPC_I2C_TypeDef;
*/

///////////////////////////////////////////////////////////////////////////////////////////////////

class I2C
{
public:

    // Configures the duty cycle
    void            SetDutyCycle(i2c_clock_mode_t mode);

protected:

    // Constructor
    I2C(i2c_port_t port);

    // Initialize I2C interface
    void            Initialize();

    // I2CONSET clear/set helper functions
    inline void     ClearSIFlag();
    inline void     ClearStartFlag();
    inline void     ClearAAFlag();
    inline void     SetStartFlag();
    inline void     SetStopFlag();
    inline void     SetAAFlag();

    // Read the status register
    uint8_t         ReadStatus();

    // Read the data register
    bool            ReadData(uint8_t &data);

    // Load data into data register
    bool            LoadData(uint8_t data);

    // Member Variables
    i2c_port_t      Port;
    LPC_I2C_TypeDef *I2CPtr;
    IRQn_Type       IRQPtr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NOT_YET_IMPLEMENTED
} PACKED i2c_master_state_t;

class I2CMaster : public I2C
{
public:

    // Constructor
    I2CMaster(i2c_port_t port);

    
    // @description            :    Master writes to a slave
    // @param slave_address    :    Address of slave being targeted
    // @param buffer           :    Pre-allocated buffer of data being sent
    // @param buffer_length    :    Length of buffer to be sent
    // @return                 :    Current state of master
    i2c_master_state_t MasterWrite( uint8_t     slave_address,
                                    char        *buffer,
                                    uint32_t    buffer_length);

    /*
        @description            :    Master reads from a slave
        @param slave_address    :    Address of slave being targeted
        @param buffer           :    Pre-allocated buffer to be filled with data
        @param buffer_length    :    Length of buffer sent
        @return                 :    Current state of master
    */
    i2c_master_state_t MasterRead(  uint8_t     slave_address,
                                    char        *buffer,
                                    uint32_t    &buffer_length);    
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// Buffer sizes
#define SLAVE_MEMORY_SIZE       (1024)
#define SLAVE_TX_BUFFER_SIZE    (512)
#define SLAVE_RX_BUFFER_SIZE    (512)

// States for slave's state machine
typedef enum 
{
    // Receiver states
    SLAVE_RX_FIRST_ADDRESSED                    = 0x60,
    SLAVE_RX_ARBITRATION_LOST                   = 0x68,
    SLAVE_RX_GENERAL_CALL                       = 0x70,
    SLAVE_RX_ARBITRATION_LOST_GENERAL_CALL      = 0x78,
    SLAVE_RX_DATA_RECEIVED_ACK                  = 0x80,
    SLAVE_RX_DATA_RECEIVED_NACK                 = 0x88,
    SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_ACK     = 0x90,
    SLAVE_RX_GENERAL_CALL_DATA_RECEIVED_NACK    = 0x98,
    SLAVE_RX_STOP_OR_REPEATED_STOP              = 0xA0,
    // Transmitter states
    SLAVE_TX_FIRST_ADDRESSED                    = 0xA8,
    SLAVE_TX_ARBITRATION_LOST                   = 0xB0,
    SLAVE_TX_DATA_TRANSMITTED_ACK               = 0xB8,
    SLAVE_TX_DATA_TRANSMITTED_NACK              = 0xC0,
    SLAVE_TX_DATA_LAST_TRANSMITTED              = 0xC8,
    // Error states
    SLAVE_NO_STATE                              = 0xF8,
    SLAVE_BUS_ERROR                             = 0x00
} 
PACKED i2c_slave_state_t;

typedef enum
{
    BUSY,
    IDLE
}
PACKED i2c_state_status_t;

// 1-4 addresses to configure I2ADR registers
typedef struct
{
    uint8_t num_addresses;  // [0, 1, 2, 3] Specify number of addresses using
    uint8_t address[4];
} 
PACKED i2c_slave_addresses_t;

typedef struct
{
    uint8_t *buffer;
    uint32_t buffer_size;
    uint32_t buffer_index;
}
PACKED buffer_t;

// Class for creating an I2C slave
class I2CSlave : public I2C
{
public:

    // Constructor, internally allocates Memory
    I2CSlave(i2c_port_t port);

    // Constructor, externally allocated Memory
    I2CSlave(i2c_port_t port, buffer_t *memory_buffer);

    // Destructor
    ~I2CSlave();

    // Initialize with slave configuration
    void                SlaveInitialize(i2c_slave_addresses_t addresses, 
                                        bool enable_general_call);

    // [ISR] Slave state machine
    i2c_state_status_t  SlaveStateMachine();

    // Loads a buffer of data into Memory
    void                LoadContiguousDataToMemory( uint8_t  *buffer, 
                                                    uint32_t memory_address, 
                                                    uint32_t buffer_length);

    // Reads a segment of data from Memory into buffer
    uint32_t            ReadContiguousDataFromMemory(uint8_t *buffer, 
                                                    uint32_t memory_address, 
                                                    uint32_t buffer_length);

    // Returns the state of the status register for debugging
    uint8_t             ReturnState();

private:

    // Loads a single byte of data into Memory
    void                LoadByteToMemory(uint8_t data, uint32_t memory_address);

    // Reads a single byte of data from Memory
    uint8_t             ReadByteFromMemory(uint32_t memory_address);

    // Helper functions to set the AA bit
    inline void         SlaveAck();
    inline void         SlaveNack();

    // Stores the current state of the state machine
    i2c_slave_state_t   SlaveCurrentState;

    // Stores the memory in which read/write operations access
    buffer_t            *Memory;
    bool                MemoryNeedsDestruction;

    // Buffers that can be filled prior to operation
    buffer_t            SlaveTxBuffer;
    buffer_t            SlaveRxBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class I2C1Slave : public I2CSlave, public SingletonTemplate <I2C1Slave>
{
private:

    I2C1Slave() : I2CSlave(I2C_PORT1) { /* EMPTY */ };

    friend class SingletonTemplate <I2C1Slave>;
};

class I2C2Slave : public I2CSlave, public SingletonTemplate <I2C2Slave>
{
private:

    I2C2Slave() : I2CSlave(I2C_PORT2) { /* EMPTY */ };

    friend class SingletonTemplate <I2C2Slave>;
};