#pragma once
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "LPC17xx.h"
#include "L4_IO/fat/disk/spi_flash.h"
#include "L4_IO/fat/disk/disk_defines.h"
#include "utilities.hpp"

typedef enum { SPI_PORT0, SPI_PORT1 } spi_port_t;
typedef enum { PCLK_DIV4, PCLK_DIV2, PCLK_DIV1, PCLK_DIV8 } pclk_divisor_t;
typedef enum { SPI_MASTER, SPI_SLAVE } spi_mode_t;

// Pins connecting to AT45
// GPIO Port 0
#define AT45_PORT	((spi_port_t)(SPI_PORT1))
#define AT45_CS 	(6)
#define AT45_SCK	(7)
#define AT45_MISO 	(8)
#define AT45_MOSI 	(9)

// Pins for other SPI
// GPIO Port 0
#define SPI_PORT	((spi_port_t)(SPI_PORT0))
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

///////////////////////////////////////////////////////////////////////////////////////////////////

class SpiBase
{
public:

	// Exchanges a byte for a byte
	byte_t 	ExchangeByte(byte_t byte, spi_mode_t mode);

	// Checks if SSP is free for operation
	bool	Busy();

	// Checks if TX FIFO ready
	bool	TxAvailable();

	// Checks if RX FIFO has data waiting
	bool	RxAvailable();

protected:

	// Constructor
	SpiBase(spi_port_t port, spi_mode_t mode);

private:

	// Initializes SPI based on port
	void 	Initialize(pclk_divisor_t divisor=PCLK_DIV1);

	// Store port for exchanging
	spi_port_t 		Port;

	// Pointer to SSP base
	LPC_SSP_TypeDef *SspPtr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class AT45DB161 : public SpiBase
{
public:

	// Constructor
	AT45DB161();

	// Reads status register, 2 bytes
	void ReadStatusRegister();

	// Reads manufacturer's ID, 1 byte
	void ReadManufacturerID();

	// Reads sector info
	void ReadSectorInfo();

	// Reads a particular sector for n number of pages
	void ReadSector(int sector, int pages);

	// Read only page 0
	void ReadPage0();

	// Not yet implemented
	void Write();

private:

	// Sets flash_cs pin low before operation
	void SetCSLow();

	// Sets flash_cs pin back to high after operation
	void SetCSHigh();
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Spi : public SpiBase
{
public:

	// Constructor
	Spi(spi_mode_t mode);

	// Sends a byte
	void 	SendByte(byte_t byte);

	// Receives a byte
	byte_t 	ReceiveByte();

	// Sends a block of bytes, make sure block is correctly allocated
	void	SendString(byte_t *block, size_t size);
	void	SendString(const char *block, size_t size);

private:

};