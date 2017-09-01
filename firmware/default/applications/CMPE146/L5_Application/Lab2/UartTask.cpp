#include <stdio.h>
#include <string>
#include "LPC17xx.h"			// LPC GPIO
#include "scheduler_task.hpp"	// scheduler_task
#include "task.h"				// Task functions, vTaskDelay
#include "sys_config.h"			// sys_get_cpu_clock

using namespace std;

#define MESSAGE  ("UART IS THE BEST PROTOCOL!")
#define BAUDRATE (115200) //(38400)

typedef enum {PORT0, PORT1, PORT2, PORT3} uart_port_t;

class UartTask : public scheduler_task
{
public:

	UartTask(uint8_t priority) : scheduler_task("UartTask", 2048, priority) {	}

	void uart_init(uart_port_t port, uint32_t baud_rate)
	{
		switch (port)
		{
			case PORT0:
				LPC_SC->PCONP 		|=  (1 << 3);
				LPC_SC->PCLKSEL0 	&= ~(3 << 6);
				LPC_SC->PCLKSEL0 	|=  (1 << 6);
				LPC_PINCON->PINSEL0 &= ~(0xF << 4);
				LPC_PINCON->PINSEL0 |=  (0x5 << 4);
				LPC_UART0->LCR 		 =  (1 << 7);
				LPC_UART0->DLM 		 = 0;
				LPC_UART0->DLL 		 = sys_get_cpu_clock() / (16 * baud_rate);
				LPC_UART0->LCR 		 = 3;
				break;

			case PORT1:
				LPC_SC->PCONP 		|=  (1 << 4);
				LPC_SC->PCLKSEL0 	&= ~(3 << 8);
				LPC_SC->PCLKSEL0 	|=  (1 << 8);
				LPC_PINCON->PINSEL0 &= ~(3 << 30);	// TXD1 is PINSEL0
				LPC_PINCON->PINSEL1 &= ~(3 <<  0);	// RXD1 is PINSEL1
				LPC_PINCON->PINSEL0 |=  (1 << 30);	// Set pins as 01 01
				LPC_PINCON->PINSEL0 |=  (1 <<  0);
				LPC_UART1->LCR 		 =  (1 << 7);
				LPC_UART1->DLM 		 = 0;
				LPC_UART1->DLL 		 = sys_get_cpu_clock() / (16 * baud_rate);
				LPC_UART1->LCR 		 = 3;
				break;

			case PORT2:
				LPC_SC->PCONP 		|=  (1 << 24);
				LPC_SC->PCLKSEL1 	&= ~(3 << 16);
				LPC_SC->PCLKSEL1 	|=  (1 << 16);
				LPC_PINCON->PINSEL0 &= ~(0xF << 20);
				LPC_PINCON->PINSEL0 |=  (0x5 << 20); // Set pins as 01 01
				LPC_UART2->LCR 		 =  (1 << 7);
				LPC_UART2->DLM 		 = 0;
				LPC_UART2->DLL 		 = sys_get_cpu_clock() / (16 * baud_rate);
				LPC_UART2->LCR 		 = 3;
				break;

			case PORT3:
				LPC_SC->PCONP 		|=  (1 << 25);
				LPC_SC->PCLKSEL1 	&= ~(3 << 18);
				LPC_SC->PCLKSEL1 	|=  (1 << 18);
				LPC_PINCON->PINSEL0 &= ~(0xF << 0);
				LPC_PINCON->PINSEL0 |=  (0xA << 0);	// Set pins as 10 10
				LPC_UART3->LCR 		 =  (1 << 7);
				LPC_UART3->DLM 		 = 0;
				LPC_UART3->DLL 		 = sys_get_cpu_clock() / (16 * baud_rate);
				LPC_UART3->LCR 		 = 3;
				break;

		}

		printf("Uart %i initialized.\n", port);
	}

	void uart_putchar(int port, char c)
	{
		switch (port)
		{
			case 0:
					LPC_UART0->THR = c;
					while ( !(LPC_UART0->LSR & (1 << 5)) );
					break;

			case 1:
					LPC_UART1->THR = c;
					while ( !(LPC_UART1->LSR & (1 << 5)) );
					break;

			case 2:
					LPC_UART2->THR = c;
					while ( !(LPC_UART2->LSR & (1 << 5)) );
					break;

			case 3:
					LPC_UART3->THR = c;
					while ( !(LPC_UART3->LSR & (1 << 5)) );
					break;

			default:
					printf("You entered a wrong port! Nothing was sent!\n");
		}
	}

	char uart_getchar(int port)
	{
		switch (port)
		{
			case 0:
				 while ( !(LPC_UART0->LSR & (1<<0)) );
				 return LPC_UART0->RBR;

			case 1:
				 while ( !(LPC_UART1->LSR & (1<<0)) );
				 return LPC_UART1->RBR;

			case 2:
				 while ( !(LPC_UART2->LSR & (1<<0)) );
				 return LPC_UART2->RBR;

			case 3:
				 while ( !(LPC_UART3->LSR & (1<<0)) );
				 return LPC_UART3->RBR;

			default:
				printf("You entered a wrong port! Returning NULL!\n");
				return (char)NULL;
		}
	}

};


////////////////////////////////////////////////////////////////////////////


class UartSendTask : public UartTask
{
public:

	UartSendTask(uint8_t priority, uart_port_t port) : UartTask(priority)
	{
		Port = port;
		msg  = MESSAGE;
		uart_init(Port, BAUDRATE);		// TX: P0.10(SDA2)		RX: P0.11(SCL2)
	}


	bool run(void *p)
	{
		printf("Sending message...\n");

		int l = msg.length();

		for (int i=0; i<l; i++) {
			uart_putchar(Port, msg[i]);
			vTaskDelay(10);
		}

		vTaskDelay(2000);
		return true;
	}

private:

	std::string 	msg;
	uart_port_t 	Port;
};


////////////////////////////////////////////////////////////////////////////


class UartEchoTask : public UartTask
{
public:

	UartEchoTask(uint8_t priority, uart_port_t port) : UartTask(priority)
	{
		Port = port;
		msg  = "";
		uart_init(Port, BAUDRATE);		// TX: P0.0		RX:	P0.1
	}


	bool run(void *p)
	{
		while (msg != MESSAGE) {
			msg += uart_getchar(3);
		}

		printf("Received message: %s\n", msg.c_str());
		msg = "";

		return true;
	}

private:

	std::string 	msg;
	uart_port_t 	Port;
};


////////////////////////////////////////////////////////////////////////////


	/*
	 *  UART INIT
	 * 	1. PCONP	= Peripheral Power Control, reset and initialize
	 * 	2. PCLKSEL 	= Peripheral Clock Selection, reset, then set as 01 to say PCLK = CCLK/1
	 * 	3. PINSEL0	= Pin function select register, reset, then set 01 01 for UART012 and 10 10 for UART3
	 * 	4. LCR		= Line Control Register, set bit 7 to enable access to divisor latches
	 * 	5. DLM		= Divisor Latch MSB Register, Table 274,
	 * 	6. DLL 		= Divisor Latch LSB Register, Table 275,
	 * 	7. LCR		= Set format of payload to be 8-bit, 1 stop bit, no parity, no break, no divisor latch
	 */

	/*
	 *  PUTCHAR
	 *  1. Send char to THR (Transmitter Holding Register), which is the top byte of the TX FIFO
	 *  2. Wait for THRE (LSR bit 5) to be empty to signify transmission is finished
	 * 		otherwise might overwrite
	 */

	/*
	 *  GETCHAR
	 *  1. Wait for char to appear in RX FIFO, waits for bit 0 to be 1, which means not empty
	 *  2. Return char from RBR which is the next received char
	 */



