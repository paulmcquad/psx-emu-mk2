#include <stdexcept>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "Ram.hpp"
#include "IOPorts.hpp"
#include "Exceptions.hpp"

constexpr unsigned int KUSEG_START    = 0x00000000;
constexpr unsigned int KSEG0_START    = 0x80000000;
constexpr unsigned int KSEG1_START    = 0xa0000000;

bool Ram::is_address_for_device(unsigned int address)
{
	if (address >= KUSEG_START && address <= KUSEG_START + MAIN_MEMORY_SIZE)
	{
		return true;
	}
	else if (address >= KSEG0_START && address <= KSEG0_START + MAIN_MEMORY_SIZE)
	{
		return true;
	}
	else if (address >= KSEG1_START && address <= KSEG1_START + MAIN_MEMORY_SIZE)
	{
		return true;
	}

	return false;
}

unsigned char Ram::get_byte(unsigned int address)
{
	return memory[address & 0x0FFFFFFF];
}

void Ram::set_byte(unsigned int address, unsigned char value)
{
	memory[address & 0x0FFFFFFF] = value;
}

void Ram::save_state(std::ofstream& file)
{
	file.write(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
}

void Ram::load_state(std::ifstream& file)
{
	file.read(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
}