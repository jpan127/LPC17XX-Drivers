#include "L5_Application/drivers/spi.hpp"
#include "L5_Application/drivers/i2c.hpp"

// XTALI: Input clock frequency

typedef enum
{
    OPCODE_READ  = 0x03,
    OPCODE_WRITE = 0x02
} vs1035b_opcode_t;

typedef enum 
{
    EAR_SPEAKER_OFF,
    EAR_SPEAKER_MINIMAL,
    EAR_SPEAKER_NORMAL,
    EAR_SPEAKER_EXTREME
} ear_speaker_mode_t;

typedef struct
{
    uint8_t  reg_num;
    bool     can_write;
    uint16_t reset_value;
    uint16_t clock_cycles;
    uint16_t reg_value;
} SCI_reg_t;

typedef enum 
{
    MODE            = 0x0,
    STATUS          = 0x1,
    BASS            = 0x2,
    CLOCKF          = 0x3,
    DECODE_TIME     = 0x4,
    AUDATA          = 0x5,
    WRAM            = 0x6,
    WRAMADDR        = 0x7,
    HDAT0           = 0x8,
    HDAT1           = 0x9,
    AIADDR          = 0xA,
    VOL             = 0xB,
    AICTRL0         = 0xC,
    AICTRL1         = 0xD,
    AICTRL2         = 0xE,
    AICTRL3         = 0xF,
    SCI_reg_last_invalid
} SCI_reg;

static SCI_reg_t SCI_reg_map[] = {
    [MODE]          = { .reg_num=MODE,        .can_write=true,  .reset_value=0x4000, .clock_cycles=80   .reg_value=0 };
    [STATUS]        = { .reg_num=STATUS,      .can_write=true,  .reset_value=0x000C, .clock_cycles=80   .reg_value=0 };
    [BASS]          = { .reg_num=BASS,        .can_write=true,  .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
    [CLOCKF]        = { .reg_num=CLOCKF,      .can_write=true,  .reset_value=0x0000, .clock_cycles=1200 .reg_value=0 };
    [DECODE_TIME]   = { .reg_num=DECODE_TIME, .can_write=true,  .reset_value=0x0000, .clock_cycles=100  .reg_value=0 };
    [AUDATA]        = { .reg_num=AUDATA,      .can_write=true,  .reset_value=0x0000, .clock_cycles=450  .reg_value=0 };
    [WRAM]          = { .reg_num=WRAM,        .can_write=true,  .reset_value=0x0000, .clock_cycles=100  .reg_value=0 };
    [WRAMADDR]      = { .reg_num=WRAMADDR,    .can_write=true,  .reset_value=0x0000, .clock_cycles=100  .reg_value=0 };
    [HDAT0]         = { .reg_num=HDAT0,       .can_write=false, .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
    [HDAT1]         = { .reg_num=HDAT1,       .can_write=false, .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
    [AIADDR]        = { .reg_num=AIADDR,      .can_write=true,  .reset_value=0x0000, .clock_cycles=210  .reg_value=0 };
    [VOL]           = { .reg_num=VOL,         .can_write=true,  .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
    [AICTRL0]       = { .reg_num=AICTRL0,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
    [AICTRL1]       = { .reg_num=AICTRL1,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
    [AICTRL2]       = { .reg_num=AICTRL2,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
    [AICTRL3]       = { .reg_num=AICTRL3,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80   .reg_value=0 };
};

typedef struct 
{
    bool FastFowardMode;
    bool RewindMode;

} vs1035b_status_t;


class VS1035b
{
public:

    // @description     : Constructor
    VS1035b();

    // @description     : Sends a single byte to the device
    // @param opcode    : Opcode to specify a read or write operation
    // @param address   : Address of register to write the data to
    // @param data      : The data byte to write
    // @returns         : True for successful, false for unsuccessful
    bool TransferSingleData(vs1035b_opcode_t opcode, uint8_t address, uint16_t data);

    // @description     : Receives a single byte from the device
    // @param opcode    : Opcode to specify a read or write operation
    // @param address   : Address of register to read the data from
    // @param data      : The data byte returned
    // @returns         : True for successful, false for unsuccessful
    bool ReceiveSingleData(vs1035b_opcode_t opcode, uint8_t address, uint16_t &data);

    // @description     : Sends an array of bytes to the device
    // @param opcode    : Opcode to specify a read or write operation
    // @param address   : The starting address of registers to write the data to
    // @param data      : Pointer to array of data to write
    // @param size      : Size of array
    // @returns         : True for successful, false for unsuccessful
    bool TransferStreamData(vs1035b_opcode_t opcode, uint8_t address, uint16_t *data, uint32_t size);

    // @description     : Receives an array of bytes to the device
    // @param opcode    : Opcode to specify a read or write operation
    // @param address   : The starting address of registers to read the data from
    // @param data      : Pointer to array of data to write
    // @param size      : Size of array
    // @returns         : True for successful, false for unsuccessful
    bool ReceiveStreamData(vs1035b_opcode_t opcode, uint8_t address, uint16_t *data, uint32_t &size);

    // @description     : Cancels the decoding if in the process of decoding
    void CancelDecoding();

    // @description     : Sets the ear speaking procecssing mode
    // @param mode      : Either off, minimal, normal, or extreme
    void SetEarSpeakerMode(ear_speaker_mode_t mode);

    // @description     : Sets the mode of streaming
    // @param on        : True for on, false for off
    void SetStreamMode(bool on);

    // @description     : Set the clock divider register to divide by 2
    // @param on        : True for on, false for off
    void SetClockDivider(bool on);

    // @description     : Get the contents of the SCI status register
    // @returns         : Contents of the status register
    uint16_t GetStatus();

    // @description     : Sets the base register value
    // @param value     : The value of the base register, possible values from 0x0 to 0xF
    void SetBaseEnhancement(uint8_t value);

    // @description     : Sets the treble register value
    // @param value     : The value of the register, possible values from 0x0 to 0xF
    // @param upper_freq: The upper frequency
    // @param lower_freq: The lower frequency
    void SetTrebleControl(uint8_t value, uint8_t upper_freq, uint8_t lower_freq);

    // @description     : Read the current decoding time register
    // @returns         : The current decoding time
    uint32_t GetCurrentDecodedTime();

    // @description       : Sets the sample rate register
    // @param sample_rate : The sample rate to set to, possible values range from 0 to 48k
    void SetSampleRate(uint16_t sample_rate);

    // @description     : Get the current sample rate
    // @returns         : The current sample rate
    uint16_t GetSampleRate();

    // @description     : Sets the volume register, left and right volumes can be different
    // @param left_vol  : Volume of the left speaker
    // @param right_vol : Volume of the right speaker
    void SetVolume(uint16_t left_vol, uint16_t right_vol);

    // @description     : Perform a hardware reset
    void HardwareReset();

    // @description     : Perform a software reset
    void SoftwareReset();

    // @description     : Turns on or off the lower power mode
    // @param on        : True for on, false for off
    void SetLowPowerMode(bool on);

    // @description     : Starts playback mode by sending an mp3 file to the device
    // @param mp3       : Array of mp3 file bytes
    // @param size      : Size of the arrray of file
    void StartPlayback(uint8_t *mp3, uint32_t size);

    // @description     : Switches current playback to another mp3 file
    // @param mp3       : Array of mp3 file bytes
    // @param size      : Size of the arrray of file
    void SwitchPlayback(uint8_t *mp3, uint32_t size);

    // @description     : Turns on or off the fast forward mode
    // @param on        : True for on, false for off
    void SetFastForwardMode(bool on);

    // @description     : Turns on or off the rewind mode
    // @param on        : True for on, false for off
    void SetRewindMode(bool on);

    // @description     : TBD state machine
    void PlaybackStateMachine();

    // @description     : Reads the current status information from the device and updates the struct
    void UpdateStatus();

    // @description     : Returns the current status struct
    // @returns         : Current status struct
    vs1035b_status_t GetStatus();

    uint32_t GetPlaybackPosition();

private:

    // Stores a struct of status information to be transmitted
    vs1035b_status_t Status;

    // @description     : Reads the endFillByte parameter from the device
    // @returns         : The endFillByte
    uint8_t GetEndFillByte();


    // @description     : Requests if the device is capable of receiving data
    // @returns         : True for available, false for not available
    inline bool Request();

    // @description     : Pull XCS line to select or deselect the device
    // @param on        : True for pull line low + select device, False for pull line high + deselect device
    inline void SwitchChipSelect(bool on);

    // @description     : Change a single bit of one of the SCI registers
    // @param register  : Specifies which SCI register
    // @param bit       : Specifies which bit of the SCI register
    // @param bit_value : Specifies value of bit to set, true for 1, false for 0
    // @returns         : True for successfully set, false for unsuccessful
    bool ChangeSCIRegister(SCI_reg register, uint8_t bit, bool bit_value);
};