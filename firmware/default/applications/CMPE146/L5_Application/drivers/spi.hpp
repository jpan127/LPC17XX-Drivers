#pragma once
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "LPC17xx.h"
#include "L4_IO/fat/disk/spi_flash.h"
#include "L4_IO/fat/disk/disk_defines.h"
#include "utilities.hpp"

typedef enum {SPI_PORT0, SPI_PORT1} spi_port_t;

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

///////////////////////////////////////////////////////////////////////////////////////////////////

class SpiBase
{
public:

	// Constructor
	SpiBase(spi_port_t port);

	// Exchanges a byte for a byte
	byte_t 	ExchangeByte(byte_t byte);

private:

	// Store port for exchanging
	spi_port_t Port;

	// Initializes SPI based on port
	void 	Initialize();
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
	Spi();

	// Sends a byte
	void 	SendByte(byte_t byte);

	// Receives a byte
	byte_t 	ReceiveByte();

	// Sends a block of bytes, make sure block is correctly allocated
	void	SendByteBlock(byte_t *block, int size);

private:

};