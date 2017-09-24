#pragma once
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <LPC17xx.h>
#include <string.h>
#include "L4_IO/fat/disk/spi_flash.h"
#include "L4_IO/fat/disk/disk_defines.h"
#include "L5_Application/drivers/utilities.hpp"
#include "L5_Application/drivers/gpio_output.hpp"

// Pins connecting to AT45
// GPIO Port 0
#define AT45_PORT	(SPI_PORT1)
#define AT45_CS 	(6)
#define AT45_SCK	(7)
#define AT45_MISO 	(8)
#define AT45_MOSI 	(9)

// Pins for other SPI
// GPIO Port 0
#define SPI_PORT	(SPI_PORT0)
#define SPI_CS		(16)
#define SPI_SCK		(15)
#define SPI_MISO	(17)
#define SPI_MOSI	(18)

// SSP Status Register
#define SPI_STATUS_TFE	(1 << 0)	// TX FIFO empty
#define SPI_STATUS_TNF	(1 << 1)	// TX FIFO not full
#define SPI_STATUS_RNE	(1 << 2)	// RX FIFO not empty
#define SPI_STATUS_RFF	(1 << 3)	// RX FIFO full
#define SPI_STATUS_BSY	(1 << 4)	// SSP busy

// Dummy byte
#define DUMMY (0x00)

typedef enum { SPI_PORT0, SPI_PORT1 } 						spi_port_t;
typedef enum { PCLK_DIV4, PCLK_DIV2, PCLK_DIV1, PCLK_DIV8 } pclk_divisor_t;
typedef enum { SPI_MASTER, SPI_SLAVE } 						spi_mode_t;

typedef struct
{
	uint8_t bootstrap_address[3];			// 0-2
	uint8_t oem_name_and_version[8];		// 3-10
	uint8_t bytes_per_sector[2];			// 11-12
	uint8_t num_sectors_per_cluster;		// 13
	uint8_t num_reserved_sectors[2];		// 14-15
	uint8_t num_fat_copies;					// 16
	uint8_t num_root_directory_entries[2];	// 17-18
	uint8_t num_total_sectors[2];			// 19-20
	uint8_t media_descriptor_type;			// 21
	uint8_t num_sectors_per_fat[2];			// 22-23
	uint8_t num_sectors_per_track[2];		// 24-25
	uint8_t num_heads[2];					// 26-27
	uint8_t num_hidden_sectors[2];			// 28-29
	uint8_t bootstrap[480];					// 30-509
	uint8_t signature[2];					// 510-511
} __attribute__((packed)) boot_sector_t;

typedef struct
{
	uint8_t bootstrap[446];
	uint8_t partition_entry1[16];
	uint8_t partition_entry2[16];
	uint8_t partition_entry3[16];
	uint8_t partition_entry4[16];
	uint8_t signature[2];
} __attribute__((packed)) master_boot_record_t;

///////////////////////////////////////////////////////////////////////////////////////////////////

class SpiBase
{
public:

	// Exchanges a byte for a byte
	byte_t 	ExchangeByte(byte_t byte);

	// Checks if SSP is free for operation
	bool	Busy();

	// Checks if TX FIFO ready
	bool	TxAvailable();

	// Checks if RX FIFO has data waiting
	bool	RxAvailable();

protected:

	// Constructor
	SpiBase(spi_port_t port, spi_mode_t mode, pclk_divisor_t divisor=PCLK_DIV1);

	// Store port for exchanging
	spi_port_t 		Port;

	// Pointer to SSP base
	LPC_SSP_TypeDef *SspPtr;

private:

	// Initializes SPI based on port
	void 	Initialize(spi_mode_t mode, pclk_divisor_t divisor);
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#define OPCODE_READ	(0x03);	// Opcode for continuous array read for f(car2) frequencies

// 16Mbit = 2Mbyte
class AT45DB161 : public SpiBase
{
public:

	// Constructor
	AT45DB161();

	// Destructor
	~AT45DB161();

	// Reads status register, 2 bytes
	void 	ReadStatusRegister();

	// Reads manufacturer's ID, 1 byte
	void 	ReadManufacturerID();

	void 	ContinuousArrayRead(int address_12bits, int byte_10bits);

	// Read a page by sending 512 dummy bytes and print to terminal
	char* 	ReadPage();

	// [TODO] Not yet implemented
	void 	Write();

private:

	// Pin for chip select
	GpioOutput ChipSelect;

	// Most recently read page
	char	*RecentlyReadPage;

	// Sets flash_cs pin low before operation
	void 	SetCSLow();

	// Sets flash_cs pin back to high after operation
	void 	SetCSHigh();

	// Print page
	void	PrintPage(char *buffer, size_t size=512);
	
	// Read only page 0
	void 	ReadPage0();

	// Read sector pointed to by LBA
	void	ReadLbaSector();
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Spi : public SpiBase
{
public:

	// Constructor
	Spi(spi_port_t port, spi_mode_t mode, gpio_port_t cs_port, gpio_pin_t cs_pin);

	// Sends a byte
	void 	SendByte(byte_t byte);

	// Receives a byte
	byte_t 	ReceiveByte();

	// Sends a block of bytes, make sure block is correctly allocated
	void	SendString(byte_t *block, size_t size);
	void	SendString(const char *block, size_t size);

private:

	// Pin for chip select
	GpioOutput ChipSelect;
};