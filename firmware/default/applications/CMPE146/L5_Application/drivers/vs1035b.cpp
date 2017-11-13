#include "vs1053b.hpp"
#include "spi.hpp"
#include <cstring>

static SCI_reg_t RegisterMap[] = {
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

static vs1053b_status_t StatusMap[] = {
    .FastFowardMode = false,
    .RewindMode     = false,
};

VS1053b::IsValidAddress(uint16_t address)
{
    if ()
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
        Spi.SendByte(OPCODE_WRITE);
        Spi.SendByte(address);
        spi.SendByte(data >> 0);
        Spi.SendByte(data >> 8);
        return true;
    }
    else
    {
        return false;
    }
}

bool VS1053b::ReceiveSingleData(uint16_t address, uint16_t *data)
{
    data = 0;

    if (IsValidAddress(address))
    {
        Spi.SendByte(OPCODE_READ);
        Spi.SendByte(address);
        data |= Spi.ReceiveByte() << 0;
        data |= Spi.ReceiveByte() << 8;
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
    if (IsValidAddress(address))
    {
        for (int i=0; i<size; i++) 
        {
            // May need some logic to determine if receivable
            ReceiveSingleData(address + i, data[i]);
        }
        return size;
    }
    else
    {
        return zero;
    }
}

void VS1053b::CancelDecoding()
{
    RegisterMap[MODE].
}

void VS1053b::UpdateRegisterMap()
{
    uint16_t data = 0;
    for (int i=0; i<SCI_reg_last_invalid; i++)
    {
        if (ReceiveSingleData(i, &data))
        {
            RegisterMap[i].reg_value = data;            
        }
        else
        {
            // Fail
        }
    }
}