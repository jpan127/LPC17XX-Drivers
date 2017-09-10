#include "utilities.hpp"

void print_bits(uint8_t c, uint8_t bits)
{
	if (bits > 8) {
		std::cout << "Too many bits to print for uint8_t." << std::endl;
		return;
	}
	for (int i=(bits-1); i>=0; i--) {
		std::cout << (c & 1);
		c >>= 1;
	}
}

void print_bits(uint16_t c, uint8_t bits)
{
	if (bits > 16) {
		std::cout << "Too many bits to print for uint16_t." << std::endl;
		return;
	}
	for (int i=(bits-1); i>=0; i--) {
		std::cout << (c & 1);
		c >>= 1;
	}
}

void print_bits(uint32_t c, uint8_t bits)
{
	if (bits > 32) {
		std::cout << "Too many bits to print for uint32_t." << std::endl;
		return;
	}
	for (int i=(bits-1); i>=0; i--) {
		std::cout << (c & 1);
		c >>= 1;
	}
}