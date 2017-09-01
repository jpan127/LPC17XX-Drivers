#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "utilities.h"
#include "io.hpp"
#include "LPC17xx.h"
#include "scheduler_task.hpp"
#include "task.h"
#include "sys_config.h"
#include "../L4_IO/fat/disk/spi_flash.h"
#include "../L4_IO/fat/disk/disk_defines.h"
using namespace std;

#define SPI_MESSAGE "SPI IS THE BEST PROTOCOL!"

class SpiTask : public scheduler_task
{
public:

	SpiTask(uint8_t priority) : scheduler_task("SpiTask", 2048, priority) {}

	void spi_init(int port)
	{
		switch (port)
		{
			case 0:
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

			case 1:
				LPC_SC->PCONP 	 	|=  (1 << 10);
				LPC_SC->PCLKSEL0 	&= ~(3 << 20);
				LPC_SC->PCLKSEL0 	|=  (1 << 20);
				LPC_PINCON->PINSEL0 &= ~( (3 << 12) | (3 << 14) | (3 << 16) | (3 << 18) );
				LPC_PINCON->PINSEL0 |=  ( (0 << 12) | (2 << 14) | (2 << 16) | (2 << 18) );
				LPC_SSP1->CR0 		 = 7;
				LPC_SSP1->CR1 		 = (1 << 1);
				LPC_SSP1->CPSR 		 = 8;							// clock prescale register, clk/8
				break;

			default:
				puts("You entered a wrong port! Initialize failed!");
				return;
		}
		printf("SPI %i initialized.\n", port);
	}

	uint8_t exchange_byte(int port, uint8_t c)
	{
		switch (port)
		{
			case 0:
				LPC_SSP0->DR = c;
				while (LPC_SSP0->SR & (1 << 4));
				return LPC_SSP0->DR;

			case 1:
				LPC_SSP1->DR = c;
				while (LPC_SSP1->SR & (1 << 4));	// wait for bit to not be BUSY
				return LPC_SSP1->DR;

			default:
				printf("You entered a wrong port! Available(0, 1). Returning NULL!");
				return (char)NULL;
		}
	}
};


class SpiSendTask : public SpiTask
{
public:

	SpiSendTask(uint8_t priority) : SpiTask(priority)
	{
		LPC_GPIO0->FIODIR |= (1 << 6);	// SET FLASH CS TO OUTPUT
		LPC_GPIO0->FIOSET |= (1 << 6);
		LPC_GPIO1->FIODIR &= ~(1 << 9);

		spi_init(1);
		msg = SPI_MESSAGE;
	}

	void send_msg()
	{
		cout << "Sending message..." << endl;

		int l = msg.length();

		for (int i=0; i<l; i++) {
			exchange_byte(1, msg[i]);
			vTaskDelay(10);
		}

		cout << "Sent message." << endl;
	}

	void read_status_reg()
	{
		LPC_GPIO0->FIOCLR |= (1 << 6);

		char c;
		c = exchange_byte(1, 0xD7);
		c = exchange_byte(1, 0x00);
		cout << "Status Register Byte 1: ";
		print_bits(c);

		if (c & (1<<0)) cout << "[Bit 0][1] Device is configured for 'power of 2' binary page size (512 bytes)." << endl;
		else 			cout << "[Bit 0][0] Device is configured for standard DataFlash page size (528 bytes)." << endl;

		if (c & (1<<1))	cout << "[Bit 1][1] Sector protection is enabled." << endl;
		else 			cout << "[Bit 1][0] Sector protection is disabled." << endl;

						cout << "[Bit 2:5] Density is 16-Mbit." << endl;

		if (c & (1<<6)) cout << "[Bit 6][1] Main memory page data does not match buffer data." << endl;
		else			cout << "[Bit 6][0] Main memory page data matches buffer data." << endl;

		if (c & (1<<7))	cout << "[Bit 7][1] Device is ready." << endl;
		else			cout << "[Bit 7][0] Device is busy with an internal operation." << endl;

		cout << "/***************************************************/" << endl;

		c = exchange_byte(1, 0x00);

		cout << "Status Register Byte 2: ";
		print_bits(c);

		if (c & (1<<0)) cout << "[Bit 0][1] A sector is erase suspended." 									<< endl;
		else 			cout << "[Bit 0][0] No sectors are erase suspended." 								<< endl;

		if (c & (1<<1))	cout << "[Bit 1][1] A sector is program suspended while using Buffer 1." 			<< endl;
		else 			cout << "[Bit 1][0] No program operation has been suspended while using Buffer 1." 	<< endl;

		if (c & (1<<2))	cout << "[Bit 2][1] A sector is program suspended while using Buffer 2." 			<< endl;
		else 			cout << "[Bit 2][0] No program operation has been suspended while using Buffer 2." 	<< endl;

		if (c & (1<<3))	cout << "[Bit 3][1] Sector Lockdown command is enabled." 							<< endl;
		else 			cout << "[Bit 3][0] Sector Lockdown command is disabled." 							<< endl;

						cout << "[Bit 4] Reserved for future use." 											<< endl;

		if (c & (1<<5))	cout << "[Bit 5][1] Erase or program error detected." 								<< endl;
		else 			cout << "[Bit 5][0] Erase or program operation was successful." 					<< endl;

						cout << "[Bit 6] Reserved for future use." 											<< endl;

		if (c & (1<<7))	cout << "[Bit 7][1] Device is ready." 												<< endl;
		else			cout << "[Bit 7][0] Device is busy with an internal operation." 					<< endl;

		LPC_GPIO0->FIOSET |= (1 << 6);

		cout << "/***************************************************/" << endl;
	}

	void print_bits(uint8_t c)
	{
		cout << "(";
		for (int i=7; i>=0; i--) {
			if ( (c & (1<<i)) >> i) cout << 1;
			else 					cout << 0;
		}
		cout << ")b";
		cout << " ----> (" << hex << uppercase << static_cast <int> (c) << ")h" << endl;
	}

	void read_manufacturerID()
	{
		uint8_t c;

		cout << "/***************************************************/" << endl;

		LPC_GPIO0->FIOCLR |= (1 << 6);

		c = exchange_byte(1, 0x9F);
		c = exchange_byte(1, 0x00);
		cout << "Manufacturer ID    : ";
		print_bits(c);

		c = exchange_byte(1, 0x00);
		cout << "Device ID (Byte 1) : ";
		print_bits(c);

		c = exchange_byte(1, 0x00);
		cout << "Device ID (Byte 2) : ";
		print_bits(c);

		LPC_GPIO0->FIOSET |= (1 << 6);

		cout << "/***************************************************/" << endl;
	}

	void read_sector_info()
	{
		flash_initialize();

		DWORD sector_count;
		flash_ioctl(GET_SECTOR_COUNT, &sector_count);
		cout << "Sector Count: " << sector_count << endl;

		WORD sector_size;
		flash_ioctl(GET_SECTOR_SIZE, &sector_size);
		cout << "Sector Size: " << sector_size << endl;

		DWORD block_size;
		flash_ioctl(GET_BLOCK_SIZE, &block_size);
		cout << "Block Size: " << block_size << endl;
	}

	void read_sector(int num, int pages)
	{
		cout << "/**************************************************************/" << endl;
		cout << "PRINTING SECTOR " << num << " : " << hex << uppercase << static_cast<int>(num) << endl;

		unsigned char page[512*pages];

		flash_initialize();

		DRESULT read_status = flash_read_sectors(page, num, 1);

		if (read_status != RES_OK) {
			cout << "DRESULT: " << read_status << endl;
			return;
		}

		else {

			for (int i=0; i<512*pages; i++) {

				if (i%2 == 0 && i>0) cout << setw(2) << " ";

				cout << hex << uppercase << static_cast<int>(page[i]);
			}
			cout << endl << endl;

		}
	}

	void read_page0()
	{
		LPC_GPIO0->FIOCLR |= (1 << 6);

		exchange_byte(1, 0xD2);
		exchange_byte(1, 0x00);	// address byte 1
		exchange_byte(1, 0x00);	// address byte 2
		exchange_byte(1, 0x00); // address byte 3
		exchange_byte(1, 0x00); // dummy   byte 1
		exchange_byte(1, 0x00); // dummy   byte 2
		exchange_byte(1, 0x00); // dummy   byte 3
		exchange_byte(1, 0x00); // dummy   byte 4

		char c;

		for (int i=0; i<512; i++) {
			c = exchange_byte(1, 0x00);
			if (i%2 == 0 && i>0) cout << setw(4) << " ";
			cout << hex << uppercase << static_cast<int>(c);
		}

		cout << endl;

	}

	bool run(void *p)
	{

//		read_manufacturerID();

//		read_status_reg();

//		read_page0();

		read_sector(0x0A, 8);

		cout << "Press SW0 to refresh." << endl;

		while (!(LPC_GPIO1->FIOPIN & (1 << 9)));	// Wait for SW0 to be pressed
		while (  LPC_GPIO1->FIOPIN & (1 << 9));		// Wait for SW0 to be released

		return true;
	}

private:
	string msg;
};


	/*
	 * 1. Set PCONP for SSPn
	 * 2. Reset PCLK, then set as CCLK/1
	 * 3. Select MISO, MOSI, and SCK function in PINSEL
	 * 4. Set CR0 = 7 which selects 8 bit format and sets SPI format
	 * 5. Set CR1 = b10 to enable SSP
	 * 6. Set CPSR(Clock Prescale Register) = 8 which sets the SCK speed to CPU/8
	 */





	/*
	 * 1. Send char out to the DR(Data Register)
	 * 2. Wait for bit 4 of the SR(Status Register) to be 0 which means not busy or is idle
	 * 3. Return char from DR
	 */


