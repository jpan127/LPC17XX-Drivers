#include "vs1053b.hpp"
#include "spi.hpp"
#include <cstring>

#define SPI     (Spi::getInstance())

static SCI_reg_t RegisterMap[] = {
    [MODE]          = { .reg_num=MODE,        .can_write=true,  .reset_value=0x4000, .clock_cycles=80,   .reg_value=0 };
    [STATUS]        = { .reg_num=STATUS,      .can_write=true,  .reset_value=0x000C, .clock_cycles=80,   .reg_value=0 };
    [BASS]          = { .reg_num=BASS,        .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    [CLOCKF]        = { .reg_num=CLOCKF,      .can_write=true,  .reset_value=0x0000, .clock_cycles=1200, .reg_value=0 };
    [DECODE_TIME]   = { .reg_num=DECODE_TIME, .can_write=true,  .reset_value=0x0000, .clock_cycles=100,  .reg_value=0 };
    [AUDATA]        = { .reg_num=AUDATA,      .can_write=true,  .reset_value=0x0000, .clock_cycles=450,  .reg_value=0 };
    [WRAM]          = { .reg_num=WRAM,        .can_write=true,  .reset_value=0x0000, .clock_cycles=100,  .reg_value=0 };
    [WRAMADDR]      = { .reg_num=WRAMADDR,    .can_write=true,  .reset_value=0x0000, .clock_cycles=100,  .reg_value=0 };
    [HDAT0]         = { .reg_num=HDAT0,       .can_write=false, .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    [HDAT1]         = { .reg_num=HDAT1,       .can_write=false, .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    [AIADDR]        = { .reg_num=AIADDR,      .can_write=true,  .reset_value=0x0000, .clock_cycles=210,  .reg_value=0 };
    [VOL]           = { .reg_num=VOL,         .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    [AICTRL0]       = { .reg_num=AICTRL0,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    [AICTRL1]       = { .reg_num=AICTRL1,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    [AICTRL2]       = { .reg_num=AICTRL2,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    [AICTRL3]       = { .reg_num=AICTRL3,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
};

static vs1053b_status_t StatusMap[] = {
    .FastFowardMode = false,
    .RewindMode     = false,
};

inline bool VS1053b::IsValidAddress(uint16_t address)
{
    bool valid = true;

    if (address < 0x1800) {
        valid = false;
    }
    else if (address > 0x18FF && address < 0x5800) {
        valid = false;
    }
    else if (address > 0x58FF && address < 0x8040) {
        valid = false;
    }
    else if (address > 0x84FF && address < 0xC000) {
        valid = false;
    }

    return valid;
}

inline bool UpdateRemoteRegister(SCI_reg reg)
{
    return TransferStreamData(RegisterMap[reg].reg_num, &RegisterMap[reg].reg_value, 1);
}

inline bool GetDreqPinValue()
{
    return Dreq.IsHigh();
}

inline void SetResetPinValue(bool value)
{
    Reset.SetValue(value);
}

inline void BlockMicroSeconds(uint16_t microseconds)
{
    MicroSecondStopWatch swatch;
    swatch.start();

    while (swatch.getElapsedTime() < microseconds);
}

VS1053b::VS1053b()
{
    // Make SPI a singleton
    // Spi spi(SPI_PORT0, SPI_MASTER, GPIO_PORT0, SPI_CS);

    // Update all the register values
    UpdateRegisterMap();

    // Update all the statuses
    UpdateStatusMap();
}

bool VS1053b::TransferSingleData(uint16_t address, uint16_t data)
{
    if (IsValidAddress(address))
    {
        SPI.SendByte(OPCODE_WRITE);
        SPI.SendByte(address);
        SPI.SendByte(data >> 8);
        SPI.SendByte(data & 0xFF);
        return true;
    }
    else
    {
        return false;
    }
}

bool VS1053b::ReceiveSingleData(uint16_t address, uint16_t *data)
{
    *data = 0;

    if (IsValidAddress(address))
    {
        SPI.SendByte(OPCODE_READ);
        SPI.SendByte(address);
        *data |= Spi.ReceiveByte() << 0;
        *data |= Spi.ReceiveByte() << 8;
        return true;
    }
    else
    {
        return false;
    }
}

bool VS1053b::TransferStreamData(uint16_t address, uint16_t *data, uint16_t size)
{
    if (IsValidAddress(address))
    {
        XDCS.SetLow();
        for (int i=0; i<size; i++)
        {
            TransferSingleData(address + i, data[i]);
            
            // Toggle XDCS every 32 bytes
            if (i > 0 && i%32 == 0)
            {
                XDCS.SetHigh();
                XDCS.SetLow();
            }
        }
        XDCS.SetHigh();
        return true;
    }
    else
    {
        return false;
    }
}

uint16_t VS1053b::ReceiveStreamData(uint16_t address, uint16_t *data, uint16_t size)
{
    memset(data, 0, sizeof(uint16_t) * size);

    XDCS.SetLow();
    for (int i=0; i<size; i++) 
    {
        // May need some logic to determine if receivable
        if (!ReceiveSingleData(address + i, data[i]))
        {
            return i;
        }

        // Toggle XDCS every 32 bytes
        if (i > 0 && i%32 == 0)
        {
            XDCS.SetHigh();
            XDCS.SetLow();
        }
    }
    XDCS.SetHigh();
    return size;
}

bool VS1053b::CancelDecoding()
{
    uint16_t data = 0;

    if (ReceiveStreamData(RegisterMap[MODE].reg_num, &data, 1))
    {
        // Update local register value
        RegisterMap[MODE].reg_value = data;
        // Set cancel bit
        RegisterMap[MODE].reg_value |= (1 << 3);
        // Update remote register value
        return UpdateRemoteRegister(MODE);
    }
    else
    {
        return false;
    }
}

inline bool VS1053b::UpdateLocalRegister(SCI_reg reg)
{
    uint16_t data = 0;

    if (ReceiveStreamData(reg, &data, 1))
    {
        RegisterMap[reg].reg_value = data;            
    }
    else
    {
        return false;
    }
}

bool VS1053b::UpdateRegisterMap()
{
    for (int i=0; i<SCI_reg_last_invalid; i++)
    {
        if (!UpdateLocalRegister(i))
        {
            return false;
        }
    }

    return true;
}

void SetEarSpeakerMode(ear_speaker_mode_t mode)
{
    UpdateLocalRegister(MODE);

    static const uint16_t low_bit  = (1 << 4);
    static const uint16_t high_bit = (1 << 7);

    switch (mode)
    {
        case EAR_SPEAKER_OFF:
            RegisterMap[MODE].reg_value &= ~low_bit;
            RegisterMap[MODE].reg_value &= ~high_bit;
            break;
        case EAR_SPEAKER_MINIMAL:
            RegisterMap[MODE].reg_value |=  low_bit;
            RegisterMap[MODE].reg_value &= ~high_bit;
            break;
        case EAR_SPEAKER_NORMAL:
            RegisterMap[MODE].reg_value &= ~low_bit;
            RegisterMap[MODE].reg_value |=  high_bit;
            break;
        case EAR_SPEAKER_EXTREME:
            RegisterMap[MODE].reg_value |=  low_bit;
            RegisterMap[MODE].reg_value |=  high_bit;
            break;
    }

    UpdateRemoteRegister(MODE);
}

void SetStreamMode(bool on)
{
    UpdateLocalRegister(MODE);

    static const uint16_t stream_bit = (1 << 6);

    if (on)
    {
        RegisterMap[MODE].reg_value |= stream_bit;
    }
    else
    {
        RegisterMap[MODE].reg_value &= ~stream_bit;
    }

    UpdateRemoteRegister(MODE);
}

void SetClockDivider(bool on)
{
    UpdateLocalRegister(MODE);

    static const uint16_t clock_range_bit = (1 << 15);

    if (on)
    {
        RegisterMap[MODE].reg_value |= clock_range_bit;
    }
    else
    {
        RegisterMap[MODE].reg_value &= ~clock_range_bit;
    }

    UpdateRemoteRegister(MODE);
}

uint16_t GetStatus()
{
    UpdateLocalRegister(STATUS);

    return RegisterMap[STATUS].reg_value;
}

void SetBaseEnhancement(uint8_t amplitude, uint8_t freq_limit)
{
    UpdateLocalRegister(STATUS);

    // Clamp to max
    if (amplitude > 0xF)  amplitude  = 0xF;
    if (freq_limit > 0xF) freq_limit = 0xF;
    
    const uint8_t bass_value = (amplitude << 4) | freq_limit;
    
    RegisterMap[STATUS].reg_value |= (bass_value << 0);

    UpdateRemoteRegister(STATUS);
}

void SetTrebleControl(uint8_t amplitude, uint8_t freq_limit)
{
    UpdateLocalRegister(STATUS);

    // Clamp to max
    if (amplitude > 0xF)  amplitude  = 0xF;
    if (freq_limit > 0xF) freq_limit = 0xF;
    
    const uint8_t treble_value = (amplitude << 4) | freq_limit;
    
    RegisterMap[STATUS].reg_value |= (treble_value << 8);

    UpdateRemoteRegister(STATUS);
}

uint16_t GetCurrentDecodedTime()
{
    UpdateLocalRegister(DECODE_TIME);

    return RegisterMap[DECODE_TIME].reg_value;
}

uint16_t GetSampleRate()
{
    UpdateLocalRegister(AUDATA);

    // If bit 0 is a 1, then the sample rate is -1, else sample rate is the same
    return (RegisterMap[AUDATA].reg_value & 1) ? (RegisterMap[AUDATA].reg_value - 1) : (RegisterMap[AUDATA].reg_value);
}

void UpdateHeaderInformation()
{
    UpdateLocalRegister(HDAT0);
    UpdateLocalRegister(HDAT1);

    // Clear header
    memset(Header, 0, sizeof(Header));

    // Copy registers to bit fields
    Header.reg1 = RegisterMap[HDAT1].reg_value;
    Header.reg0 = RegisterMap[HDAT0].reg_value;

    // HDAT1
    Header.stream_valid = (Header.reg1.sync_word) == 2047;
    Header.id           = Header.reg1.id;
    Header.layer        = Header.reg1.layer;
    Header.protect_bit  = Header.reg1.protect_bit;

    // HDAT0
    Header.pad_bit      = Header.reg0.pad_bit;
    Header.mode         = Header.reg0.mode;

    // Lookup sample rate
    switch (Header.reg0.sample_rate)
    {
        case 3:
            // No sample rate
            break;
        case 2:
            switch (layer)
            {
                case 3:  Header.sample_rate = 32000; break;
                case 2:  Header.sample_rate = 16000; break;
                default: Header.sample_rate =  8000; break;
            }
        case 1:
            switch (layer)
            {
                case 3:  Header.sample_rate = 48000; break;
                case 2:  Header.sample_rate = 24000; break;
                default: Header.sample_rate = 12000; break;
            }
        case 0:
            switch (layer)
            {
                case 3:  Header.sample_rate = 44100; break;
                case 2:  Header.sample_rate = 22050; break;
                default: Header.sample_rate = 11025; break;
            }
    }

    // Calculate bit rate
    uint16_t increment_value = 0;
    uint16_t start_value     = 0;
    switch (layer)
    {
        case 1: 
            switch (id)
            {
                case 3:  increment_value = 32; start_value = 32; break;
                default: increment_value = 8;  start_value = 32; break;
            }
            break;
        case 2: 
            switch (id)
            {
                case 3:  increment_value = 16; start_value = 32; break;
                default: increment_value = 8;  start_value = 8;  break;
            }
            break;
        case 3: 
            switch (id)
            {
                case 3:  increment_value = 8; start_value = 32; break;
                default: increment_value = 8; start_value = 8;  break;
            }
            break;
    }

    const uint16_t bits_in_kilobit = (1 << 10);
    if (Header.reg0.bit_rate != 0 && Header.reg0.bit_rate != 0xF)
    {
        Header.bit_rate = (start_value + (increment_value * (Header.reg0.bit_rate-1))) * bits_in_kilobit;
    }
}

mp3_header_t GetHeaderInformation()
{
    return Header;
}

uint16_t GetBitRate()
{
    UpdateHeaderInformation();

    return Header.bit_rate;
}

void SetVolume(uint8_t left_vol, uint8_t right_vol)
{
    uint16_t volume = (left_vol << 8) | right_vol;
    RegisterMap[VOL].reg_value = volume;

    UpdateRemoteRegister(VOL);
}

void HardwareReset()
{
    // Pull reset line low
    SetResetPinValue(false);

    // Wait 1 ms, should wait shorter if possible
    vTaskDelay(1 / portTICK_PERIOD_MS);

    // Pull reset line back high
    SetResetPinValue(true);

    // Wait for 3 us at a time until DREQ goes high
    while (!GetDreqPinValue())
    {
        BlockMicroSeconds(3);
    }
}

void SoftwareReset()
{
    UpdateLocalRegister(MODE);

    // Set reset bit
    RegisterMap[MODE].reg_value |= (1 << 2);
    UpdateRemoteRegister(MODE);

    // Wait for 3 us at a time until DREQ goes high
    while (!GetDreqPinValue())
    {
        BlockMicroSeconds(3);
    }

    // Unset reset bit
    RegisterMap[MODE].reg_value &= ~(1 << 2);
    UpdateRemoteRegister(MODE);
}

void SetLowPowerMode(bool on)
{
    if (on)
    {
        // If not already in low power mode
        if (!Status.low_power_mode)
        {
            // Set clock speed to 1.0x, disabling PLL
            RegisterMap[CLOCKF].reg_value = 0x0000;
            UpdateRemoteRegister(CLOCKF);

            // Reduce sample rate
            RegisterMap[AUDATA].reg_value = 0x0010;
            UpdateRemoteRegister(AUDATA);

            // Turn off ear speaker mode
            SetEarSpeakerMode(EAR_SPEAKER_OFF);

            // Turn off analog drivers
            SetVolume(0xFFFF);
        }
    }
    else
    {

        // If in low power mode
        if (Status.low_power_mode)
        {
            // Turn off analog drivers
            SetVolume(0xFFFF);

            // Turn off ear speaker mode
            SetEarSpeakerMode(EAR_SPEAKER_OFF);

            // Reduce sample rate
            RegisterMap[AUDATA].reg_value = 0x0010;
            UpdateRemoteRegister(AUDATA);

            // Set clock speed to 1.0x, disabling PLL
            RegisterMap[CLOCKF].reg_value = 0x0000;
            UpdateRemoteRegister(CLOCKF);
        }
    }
}

void StartPlayback(uint8_t *mp3, uint32_t size)
{
    TransferStreamData(address, mp3, size);
}