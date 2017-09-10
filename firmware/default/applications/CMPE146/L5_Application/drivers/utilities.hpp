#pragma once
#include <iostream>

// Commonly used typedefs
typedef unsigned char 	uchar;
typedef unsigned int	uint;
typedef uint8_t			byte_t;

// Prints bits of an 8-bit variable
void print_bits(uint8_t c, uint8_t bits=8);

// Prints bits of an 16-bit variable
void print_bits(uint16_t c, uint8_t bits=16);

// Prints bits of an 32-bit variable
void print_bits(uint32_t c, uint8_t bits=32);