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
        SPI.SendByte(data >> 0);
        SPI.SendByte(data >> 8);
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
        for (int i=0; i<size; i++)
        {
            TransferSingleData(address + i, data[i]);
        }
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

    for (int i=0; i<size; i++) 
    {
        // May need some logic to determine if receivable
        if (!ReceiveSingleData(address + i, data[i]))
        {
            return i;
        }
    }
    return size;
}

bool VS1053b::CancelDecoding()
{
    uint16_t data = 0;

    if (ReceiveSingleData(RegisterMap[MODE].reg_num, &data))
    {
        // Update local register value
        RegisterMap[MODE].reg_value = data;
        // Set cancel bit
        RegisterMap[MODE].reg_value |= (1 << 3);
        // Update remote register value
        return TransferSingleData(RegisterMap[MODE].reg_num, RegisterMap[MODE].reg_value)
    }
    else
    {
        return false;
    }
}

bool VS1053b::UpdateRegister(SCI_reg reg)
{
    uint16_t data = 0;

    if (ReceiveSingleData(reg, &data))
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
        if (!UpdateRegister(i))
        {
            return false;
        }
    }

    return true;
}

void SetEarSpeakerMode(ear_speaker_mode_t mode)
{
    UpdateRegister(MODE);

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

    TransferSingleData(RegisterMap[MODE].reg_num, RegisterMap[MODE].reg_value);
}

void SetStreamMode(bool on)
{
    UpdateRegister(MODE);

    static const uint16_t stream_bit = (1 << 6);

    if (on)
    {
        RegisterMap[MODE].reg_value |= stream_bit;
    }
    else
    {
        RegisterMap[MODE].reg_value &= ~stream_bit;
    }

    TransferSingleData(RegisterMap[MODE].reg_num, RegisterMap[MODE].reg_value);
}

void SetClockDivider(bool on)
{
    UpdateRegister(MODE);

    static const uint16_t clock_range_bit = (1 << 15);

    if (on)
    {
        RegisterMap[MODE].reg_value |= clock_range_bit;
    }
    else
    {
        RegisterMap[MODE].reg_value &= ~clock_range_bit;
    }

    TransferSingleData(RegisterMap[MODE].reg_num, RegisterMap[MODE].reg_value);
}

uint16_t GetStatus()
{
    UpdateRegister(STATUS);

    return RegisterMap[STATUS].reg_value;
}

void SetBaseEnhancement(uint8_t amplitude, uint8_t freq_limit)
{
    UpdateRegister(STATUS);

    // Clamp to max
    if (amplitude > 0xF)  amplitude  = 0xF;
    if (freq_limit > 0xF) freq_limit = 0xF;
    
    const uint8_t bass_value = (amplitude << 4) | freq_limit;
    
    RegisterMap[STATUS].reg_value |= (bass_value << 0);

    TransferSingleData(RegisterMap[STATUS].reg_num, RegisterMap[STATUS].reg_value);
}

void SetTrebleControl(uint8_t amplitude, uint8_t freq_limit)
{
    UpdateRegister(STATUS);

    // Clamp to max
    if (amplitude > 0xF)  amplitude  = 0xF;
    if (freq_limit > 0xF) freq_limit = 0xF;
    
    const uint8_t treble_value = (amplitude << 4) | freq_limit;
    
    RegisterMap[STATUS].reg_value |= (treble_value << 8);

    TransferSingleData(RegisterMap[STATUS].reg_num, RegisterMap[STATUS].reg_value);
}