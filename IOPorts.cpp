#include "IOPorts.hpp"
#include <typeinfo.h>

unsigned char* IOPorts::operator[](unsigned int addr)
{
	return &data[addr];
}

unsigned char* IOPorts::operator[](io_registers io_reg)
{
	unsigned int base_address = static_cast<unsigned int>(io_registers::EXPANSION_1_BASE_ADDRESS);
	unsigned int address = static_cast<unsigned int>(io_reg) - base_address;
	return &data[address];
}