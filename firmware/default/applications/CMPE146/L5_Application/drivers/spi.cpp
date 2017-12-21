#include "spi.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////

//
// 1. Set PCONP for SSPn
// 2. Reset PCLK, then set as CCLK/1
// 3. Select MISO, MOSI, and SCK function in PINSEL
// 4. Set CR0 = 7 which selects 8 bit format and sets SPI format
// 5. Set CR1 = b10 to enable SSP
// 6. Set CPSR(Clock Prescale Register) = 8 which sets the SCK speed to CPU/8
//

//
// 1. Send char out to the DR(Data Register)
// 2. Wait for bit 4 of the SR(Status Register) to be 0 which means not busy or is idle
// 3. Return char from DR
//

SpiBase::SpiBase(spi_port_t port, spi_mode_t mode, pclk_divisor_t divisor)
{
    Port = port;

    switch (Port)
    {
        case SPI_PORT0: SspPtr = LPC_SSP0; break;
        case SPI_PORT1: SspPtr = LPC_SSP1; break;
    }

    Initialize(mode, divisor);
}

void SpiBase::Initialize(spi_mode_t mode, pclk_divisor_t divisor)
{
    uint8_t divisor_setting = 1;
    switch (divisor)
    {
        case PCLK_DIV4: divisor_setting = 0x0; break;
        case PCLK_DIV2: divisor_setting = 0x2; break;
        case PCLK_DIV1: divisor_setting = 0x1; break;
        case PCLK_DIV8: divisor_setting = 0x3; break;
    }

    switch (Port)
    {
        case SPI_PORT0:

            LPC_SC->PCONP       |=  (1 << 21);
            LPC_SC->PCLKSEL1    &= ~(3 << 10);
            LPC_SC->PCLKSEL1    |=  (divisor_setting << 10);
            // SCK0 is in PINSEL0
            // MISO0 MOSI0 is in PINSEL1
            LPC_PINCON->PINSEL0 &= ~(3 << 30);
            LPC_PINCON->PINSEL1 &= ~( (3 << 0) | (3 << 2) | (3 << 4) ); 
            LPC_PINCON->PINSEL0 |=  (2 << 30);
            LPC_PINCON->PINSEL1 |=  ( (2 << 0) | (2 << 2) | (2 << 4) );
            break;

        case SPI_PORT1:

            LPC_SC->PCONP       |=  (1 << 10);
            LPC_SC->PCLKSEL0    &= ~(3 << 20);
            LPC_SC->PCLKSEL0    |=  (divisor_setting << 20);
            LPC_PINCON->PINSEL0 &= ~( (3 << 12) | (3 << 14) | (3 << 16) | (3 << 18) );
            LPC_PINCON->PINSEL0 |=  ( (2 << 12) | (2 << 14) | (2 << 16) | (2 << 18) );
            break;
    }

    // 8 bit data transfer, spi frame format, 
    SspPtr->CR0  = 0x7;
    // Bit 1 is SSP enable, Bit 2 determines if master (value 0) or slave (value 1)
    SspPtr->CR1  = (mode == SPI_MASTER) ? (1 << 1) : ( (1 << 1) | (1 << 2) );
    // Clock prescale register
    // PCLK / (CPSR * (SCR=0 + 1))
    SspPtr->CPSR = 36; // 48Mhz / 4 = 12Mhz

    printf("SPI %i initialized.\n", Port);
}

byte_t SpiBase::ExchangeByte(byte_t byte)
{
    // Wait until TX FIFO is not full
    while ( !TxAvailable() );
    // Put in a byte
    SspPtr->DR = byte;
    // Wait until SSP not busy
    while ( Busy() );

    // Wait until RX FIFO not empty
    while ( !RxAvailable() );
    // Return exchanged byte
    return SspPtr->DR;
    // Wait until SSP not busy
    while ( Busy() );
}

bool SpiBase::Busy()
{
    // Returns true if busy
    return ( SspPtr->SR & SPI_STATUS_BSY );
}

bool SpiBase::TxAvailable()
{
    // Returns true if not full
    return ( SspPtr->SR & SPI_STATUS_TNF );
}

bool SpiBase::RxAvailable()
{
    // Returns true if not empty
    return ( SspPtr->SR & SPI_STATUS_RNE );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

AT45DB161::AT45DB161() : SpiBase(SPI_PORT1, SPI_MASTER), ChipSelect(GPIO_PORT0, 6)
{
    LPC_GPIO0->FIODIR |= (1 << 0);
    RecentlyReadPage = new char[512];
}

AT45DB161::~AT45DB161()
{
    delete [] RecentlyReadPage;
}

void AT45DB161::ReadStatusRegister()
{
    SetCSLow();

    puts("///////////////////////////////////////////////////////////////////////////////////");
    printf("\n");

    uint8_t c;
    c = ExchangeByte(0xD7);
    c = ExchangeByte(0x00);
    
    printf("Status Register Byte 1: "); print_bits(c, 8); printf(" | 0x%02X\n", c);

    if (c & (1<<0)) {puts("[Bit 0][1] Device is configured for 'power of 2' binary page size (512 bytes).");}
    else            {puts("[Bit 0][0] Device is configured for standard DataFlash page size (528 bytes).");}

    if (c & (1<<1)) {puts("[Bit 1][1] Sector protection is enabled.");}
    else            {puts("[Bit 1][0] Sector protection is disabled.");}

                    puts("[Bit 2:5]  Density is 16-Mbit.");

    if (c & (1<<6)) {puts("[Bit 6][1] Main memory page data does not match buffer data.");}
    else            {puts("[Bit 6][0] Main memory page data matches buffer data.");}

    if (c & (1<<7)) {puts("[Bit 7][1] Device is ready.");}
    else            {puts("[Bit 7][0] Device is busy with an internal operation.");}

    puts("///////////////////////////////////////////////////////////////////////////////////");

    c = ExchangeByte(0x00);

    printf("Status Register Byte 2: "); print_bits(c, 8); printf(" | 0x%02X\n", c);

    if (c & (1<<0)) {puts("[Bit 0][1] A sector is erase suspended.");}
    else            {puts("[Bit 0][0] No sectors are erase suspended.");}

    if (c & (1<<1)) {puts("[Bit 1][1] A sector is program suspended while using Buffer 1.");}
    else            {puts("[Bit 1][0] No program operation has been suspended while using Buffer 1.");}

    if (c & (1<<2)) {puts("[Bit 2][1] A sector is program suspended while using Buffer 2.");}
    else            {puts("[Bit 2][0] No program operation has been suspended while using Buffer 2.");}

    if (c & (1<<3)) {puts("[Bit 3][1] Sector Lockdown command is enabled.");}
    else            {puts("[Bit 3][0] Sector Lockdown command is disabled.");}

                    puts("[Bit 4]    Reserved for future use.");

    if (c & (1<<5)) {puts("[Bit 5][1] Erase or program error detected.");}
    else            {puts("[Bit 5][0] Erase or program operation was successful.");}

                    puts("[Bit 6]    Reserved for future use.");

    if (c & (1<<7)) {puts("[Bit 7][1] Device is ready.");}
    else            {puts("[Bit 7][0] Device is busy with an internal operation.");}

    printf("\n");

    SetCSHigh();
}

void AT45DB161::ReadManufacturerID()
{
    SetCSLow();

    puts("///////////////////////////////////////////////////////////////////////////////////");
    printf("\n");

    byte_t c;
    c = ExchangeByte(0x9F);

    c = ExchangeByte(0x00);
    printf("Manufacturer ID    : "); print_bits(c, 8); printf(" | 0x%02X\n", c);

    c = ExchangeByte(0x00);
    printf("Device ID (Byte 1) : "); print_bits(c, 8); printf(" | 0x%02X\n", c);

    c = ExchangeByte(0x00);
    printf("Device ID (Byte 2) : "); print_bits(c, 8); printf(" | 0x%02X\n", c);

    printf("\n");

    SetCSHigh();
}

void AT45DB161::ContinuousArrayRead(uint32_t address_12bits, uint32_t byte_10bits)
{
    // Mask the other 20 bits
    uint32_t address = address_12bits & 0x00000FFF;
    // Mask the other 22 bits
    uint32_t byte    = byte_10bits & 0x00000FFF;
    byte            &= ~(0x3 << 10);
    // Combine into 22 bits
    address          = (address << 10) | byte;
    // Split up into 3 bytes
    char byte1       = (char)(address >> 16);
    char byte2       = (char)(address >> 8);
    char byte3       = (char)(address >> 0);

    SetCSLow();

    ExchangeByte(AT45_OPCODE_READ);
    // First 12 bits [11:0] specify the page
    // Last  10 bits [22:12] specify the starting byte address in the page
    ExchangeByte(byte1);
    ExchangeByte(byte2);
    ExchangeByte(byte3);

    char *buffer = ReadPage();
    PrintPage(buffer, 512);
    delete [] buffer;

    SetCSHigh();
}

void AT45DB161::ReadPage0()
{
    SetCSLow();

    ExchangeByte(AT45_OPCODE_READ);
    // First 12 bits [11:0] specify the page
    // Last  10 bits [22:12] specify the starting byte address in the page
    ExchangeByte(0x00); // Address 1
    ExchangeByte(0x00); // Address 2
    ExchangeByte(0x00); // Address 3

    char *buffer = ReadPage();
    PrintPage(buffer, 512);
    delete [] buffer;

    SetCSHigh();
}

void AT45DB161::ReadLbaSector()
{
    SetCSLow();

    ExchangeByte(AT45_OPCODE_READ);
    ExchangeByte(0x00); // Address 1
    ExchangeByte(0x00); // Address 2
    ExchangeByte(0x00); // Address 3

    char *buffer = ReadPage();
    PrintPage(buffer, 512);

    printf("\n\n");
    printf("LBA: \n");
    printf("%02X ", buffer[446+8]);
    printf("%02X ", buffer[446+9]);
    printf("%02X ", buffer[446+10]);
    printf("%02X ", buffer[446+11]);
    printf("\n\n");

    ExchangeByte(0x03);
    ExchangeByte(buffer[446+10]);
    ExchangeByte(buffer[446+9]);
    ExchangeByte(buffer[446+8]);

    buffer = ReadPage();
    PrintPage(buffer, 512);
    delete [] buffer;

    SetCSHigh();
}

char* AT45DB161::ReadPage()
{
    char *buffer = new char[512];

    for (int i=0; i<512; i++) {
        buffer[i] = ExchangeByte(DUMMY);
    }

    return buffer;
}

void AT45DB161::PrintPage(char *buffer, size_t size)
{
    for (size_t i=0; i<size; i++) {
        // New line
        if (i%16 == 0 && i>0) {
            std::cout << std::endl;
        }
        // Space every 2 bytes
        else if (i%2 == 0 && i>0) {
            std::cout << " ";
        }
        std::cout << std::setw(5) << std::hex << std::uppercase << (int)buffer[i];
    }
}

void AT45DB161::SetCSLow()
{
    ChipSelect.SetLow();
    LPC_GPIO0->FIOCLR |= (1 << 0);
}

void AT45DB161::SetCSHigh()
{
    ChipSelect.SetHigh();
    LPC_GPIO0->FIOSET |= (1 << 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Spi::Spi(spi_port_t port, spi_mode_t mode, gpio_port_t cs_port, gpio_pin_t cs_pin) : 
                                                        SpiBase(port, mode)
{
    /* EMPTY */
}

void Spi::SendByte(byte_t byte)
{
    ExchangeByte(byte);
}

byte_t Spi::ReceiveByte()
{
    return ExchangeByte(0x00);
}

void Spi::SendString(byte_t *block, size_t size)
{
    for (size_t i=0; i<size; i++) {
        SendByte(block[i]);
    }
}

void Spi::SendString(const char *block, size_t size)
{
    for (size_t i=0; i<size; i++) {
        SendByte(block[i]);
    }   
}