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

    // Constructor
    VS1035b();

    //
    bool TransferSingleData(vs1035b_opcode_t opcode, uint8_t address, uint16_t data);

    //
    bool ReceiveSingleData(vs1035b_opcode_t opcode, uint8_t address, uint16_t &data);

    //
    bool TransferStreamData(vs1035b_opcode_t opcode, uint8_t address, uint16_t data);

    //
    bool ReceiveStreamData(vs1035b_opcode_t opcode, uint8_t address, uint16_t data);

    //
    void CancelDecoding();

    void SetEarSpeakerMode(ear_speaker_mode_t mode);

    void SetStreamMode(bool on);

    void SetClockDivider(bool on);

    uint16_t GetStatus(bool print);

    void SetBaseEnhancement(uint8_t value);

    void SetTrebleControl(uint8_t value, uint8_t upper_frequency, uint8_t lower_frequency);

    uint32_t GetCurrentDecodedTime();

    void SetSampleRate(uint16_t sample_rate);

    uint16_t GetSampleRate();

    void SetVolume(uint16_t left_vol, uint16_t right_vol);

    void HardwareReset();

    void SoftwareReset();

    void SetLowPowerMode(bool on);

    void StartPlayback(uint8_t *mp3, uint32_t size);

    void SwitchPlayback(uint8_t *mp3, uint32_t size);

    void SetFastForwardMode(bool on);

    void SetRewindMode(bool on);

    void PlaybackStateMachine();

    void UpdateStatus();

    vs1035b_status_t GetStatus();

private:

    vs1035b_status_t Status;

    // Requests if the device is capable of receiving data
    // @returns:    True for available, false for not available
    inline bool Request();

    // Pull XCS line to select or deselect the device
    // @param on:   True for pull line low + select device, False for pull line high + deselect device
    inline void SwitchChipSelect(bool on);

    // @description     : Change a single bit of one of the SCI registers
    // @param register  : Specifies which SCI register
    // @param bit       : Specifies which bit of the SCI register
    // @param bit_value : Specifies value of bit to set, true for 1, false for 0
    // @returns         : True for successfully set, false for unsuccessful
    bool ChangeSCIRegister(SCI_reg register, uint8_t bit, bool bit_value);

    bool ChangeMode(uint8_t bit, bool bit_value);
};