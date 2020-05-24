#include <stdexcept>
#include <iostream>
#include <sstream>
#include <assert.h>
#include "Ram.hpp"

bool Ram::is_address_for_device(unsigned int address)
{
	if (address >= 0x0 && address < MAIN_MEMORY_SIZE)
	{
		return true;
	}
	else if (address >= 0x80000000 && address <= 0x80000000 + MAIN_MEMORY_SIZE)
	{
		return true;
	}
	else if (address >= 0xa0000000 && address <= 0xa0000000 + MAIN_MEMORY_SIZE)
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

void Ram::save_state(std::stringstream& file)
{
	file.write(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
}

void Ram::load_state(std::stringstream& file)
{
	file.read(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
}