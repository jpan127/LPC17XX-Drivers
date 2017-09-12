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

SpiBase::SpiBase(spi_port_t port)
{
	Port = port;

	Initialize();
}

void SpiBase::Initialize()
{
	switch (Port)
	{
		case SPI_PORT0:
			LPC_SC->PCONP 	 	|=  (1 << 21);
			LPC_SC->PCLKSEL1 	&= ~(3 << 10);
			LPC_SC->PCLKSEL1 	|=  (1 << 10);
			LPC_PINCON->PINSEL0 &= ~(3 << 30); 					// SCK0 is in PINSEL0
			LPC_PINCON->PINSEL1 &= ~( (3 << 2) | (3 << 4) );	// MISO0 MOSI0 is in PINSEL1
			LPC_PINCON->PINSEL0 |=  (2 << 30);
			LPC_PINCON->PINSEL1 |=  ( (2 << 2) | (2 << 4) );
			LPC_SSP0->CR0 		 = 7;
			LPC_SSP0->CR1 		 = (1 << 1);
			LPC_SSP0->CPSR 		 = 8;
			break;

		case SPI_PORT1:
			LPC_SC->PCONP 	 	|=  (1 << 10);
			LPC_SC->PCLKSEL0 	&= ~(3 << 20);
			LPC_SC->PCLKSEL0 	|=  (1 << 20);
			LPC_PINCON->PINSEL0 &= ~( (3 << 12) | (3 << 14) | (3 << 16) | (3 << 18) );
			LPC_PINCON->PINSEL0 |=  ( (0 << 12) | (2 << 14) | (2 << 16) | (2 << 18) );
			LPC_SSP1->CR0 		 = 7;
			LPC_SSP1->CR1 		 = (1 << 1);
			LPC_SSP1->CPSR 		 = 8;							// clock prescale register, clk/8
			break;
	}

	printf("SPI %i initialized.\n", Port);
}

byte_t SpiBase::ExchangeByte(byte_t byte)
{
	switch (Port)
	{
		case SPI_PORT0:
			LPC_SSP0->DR = byte;
			while (LPC_SSP0->SR & (1 << 4));
			return LPC_SSP0->DR;

		case SPI_PORT1:
			LPC_SSP1->DR = byte;
			while (LPC_SSP1->SR & (1 << 4));	// wait for bit to not be BUSY
			return LPC_SSP1->DR;
	}

	// Should never reach
	return (byte_t)NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

AT45DB161::AT45DB161() : SpiBase(SPI_PORT1)
{
	// Set flash_cs as output
	LPC_GPIO0->FIODIR |= (1 << 6);
	LPC_GPIO0->FIOSET |= (1 << 6);
	// Set SW0 as input
	LPC_GPIO1->FIODIR &= ~(1 << 9);
}

void AT45DB161::ReadStatusRegister()
{
	SetCSLow();

	uint8_t c;
	c = ExchangeByte(0xD7);
	c = ExchangeByte(0x00);
	
	printf("Status Register Byte 1: "); print_bits(c);

	if (c & (1<<0)) puts("[Bit 0][1] Device is configured for 'power of 2' binary page size (512 bytes).");
	else 			puts("[Bit 0][0] Device is configured for standard DataFlash page size (528 bytes).");

	if (c & (1<<1))	puts("[Bit 1][1] Sector protection is enabled.");
	else 			puts("[Bit 1][0] Sector protection is disabled.");

					puts("[Bit 2:5]  Density is 16-Mbit.");

	if (c & (1<<6)) puts("[Bit 6][1] Main memory page data does not match buffer data.");
	else			puts("[Bit 6][0] Main memory page data matches buffer data.");

	if (c & (1<<7))	puts("[Bit 7][1] Device is ready.");
	else			puts("[Bit 7][0] Device is busy with an internal operation.");

	puts("///////////////////////////////////////////////////////////////////////////////////");

	c = ExchangeByte(0x00);

	printf("Status Register Byte 2: "); print_bits(c, 8);

	if (c & (1<<0)) puts("[Bit 0][1] A sector is erase suspended.");
	else 			puts("[Bit 0][0] No sectors are erase suspended.");

	if (c & (1<<1))	puts("[Bit 1][1] A sector is program suspended while using Buffer 1.");
	else 			puts("[Bit 1][0] No program operation has been suspended while using Buffer 1.");

	if (c & (1<<2))	puts("[Bit 2][1] A sector is program suspended while using Buffer 2.");
	else 			puts("[Bit 2][0] No program operation has been suspended while using Buffer 2.");

	if (c & (1<<3))	puts("[Bit 3][1] Sector Lockdown command is enabled.");
	else 			puts("[Bit 3][0] Sector Lockdown command is disabled.");

					puts("[Bit 4]    Reserved for future use.");

	if (c & (1<<5))	puts("[Bit 5][1] Erase or program error detected.");
	else 			puts("[Bit 5][0] Erase or program operation was successful.");

					puts("[Bit 6]    Reserved for future use.");

	if (c & (1<<7))	puts("[Bit 7][1] Device is ready.");
	else			puts("[Bit 7][0] Device is busy with an internal operation.");

	puts("///////////////////////////////////////////////////////////////////////////////////");

	SetCSHigh();
}

void AT45DB161::ReadManufacturerID()
{
	SetCSLow();

	puts("///////////////////////////////////////////////////////////////////////////////////");

	byte_t c;
	c = ExchangeByte(0x9F);

	c = ExchangeByte(0x00);
	printf("Manufacturer ID    : "); print_bits(c, 8); puts("");

	c = ExchangeByte(0x00);
	printf("Device ID (Byte 1) : "); print_bits(c, 8); puts("");

	c = ExchangeByte(0x00);
	printf("Device ID (Byte 2) : "); print_bits(c, 8); puts("");

	puts("///////////////////////////////////////////////////////////////////////////////////");

	SetCSHigh();
}

void AT45DB161::ReadSectorInfo()
{
	// Initialize the flash memory
	flash_initialize();

	// Get sector count
	DWORD sector_count;
	flash_ioctl(GET_SECTOR_COUNT, &sector_count);		// Get control information
	printf("Sector Count: %lu\n", sector_count);		// unsigned long

	// Get sector size
	WORD sector_size;
	flash_ioctl(GET_SECTOR_SIZE, &sector_size);			// Get control information
	printf("Sector Size: %i\n", sector_size);			// unsigned short

	// Get block size
	DWORD block_size;
	flash_ioctl(GET_BLOCK_SIZE, &block_size);			// Get control information
	printf("Block Size: %lu\n", block_size);			// unsigned long
}

void AT45DB161::ReadSector(int sector, int pages)
{
	puts("///////////////////////////////////////////////////////////////////////////////////");
	printf("Printing Sector %i\n", sector);

	unsigned char page[512*pages];

	flash_initialize();

	DRESULT read_status = flash_read_sectors(page, sector, 1);

	if (read_status != RES_OK) 
	{
		printf("DRESULT: %i\n", read_status);
	}
	else 
	{
		for (int i=0; i<512*pages; i++) {
			// Format print block of values
			if (i%2 == 0 && i>0) std::cout << std::setw(2) << " ";
			std::cout << std::hex << std::uppercase << static_cast<int>(page[i]);
		}
		printf("\n\n");
	}
}

void AT45DB161::ReadPage0()
{
	SetCSLow();

	ExchangeByte(0xD2);
	ExchangeByte(0x00);	// address byte 1
	ExchangeByte(0x00);	// address byte 2
	ExchangeByte(0x00); // address byte 3
	ExchangeByte(0x00); // dummy   byte 1
	ExchangeByte(0x00); // dummy   byte 2
	ExchangeByte(0x00); // dummy   byte 3
	ExchangeByte(0x00); // dummy   byte 4

	char c;

	for (int i=0; i<512; i++) {
		c = ExchangeByte(0x00);
		if (i%2 == 0 && i>0) std::cout << std::setw(4) << " ";
		std::cout << std::hex << std::uppercase << static_cast<int>(c);
	}

	puts("");

	SetCSHigh();
}

void AT45DB161::SetCSLow()
{
	LPC_GPIO0->FIOCLR |= (1 << 6);
}

void AT45DB161::SetCSHigh()
{
	LPC_GPIO0->FIOSET |= (1 << 6);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Spi::Spi() : SpiBase(SPI_PORT0)
{
	// Set nordic_cs as output
	LPC_GPIO0->FIODIR |= (1 << 16);
	LPC_GPIO0->FIOSET |= (1 << 16);
	// Set SW0 as input
	LPC_GPIO1->FIODIR &= ~(1 << 9);
}

void Spi::SendByte(byte_t byte)
{
	ExchangeByte(byte);
}

byte_t Spi::ReceiveByte()
{
	return ExchangeByte(0x00);
}

void Spi::SendByteBlock(byte_t *block, int size)
{
	for (int i=0; i<size; i++) {
		SendByte(block[i]);
	}
}