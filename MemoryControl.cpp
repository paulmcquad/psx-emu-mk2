#include "MemoryControl.hpp"

constexpr unsigned int MEMORY_CONTROL_1_SIZE = 36;
constexpr unsigned int MEMORY_CONTROL_2_SIZE = 4;

constexpr unsigned int MEMORY_CONTROL_1_START = 0x1F801000;
constexpr unsigned int MEMORY_CONTROL_1_END = MEMORY_CONTROL_1_START + MEMORY_CONTROL_1_SIZE;

constexpr unsigned int MEMORY_CONTROL_2_START = 0x1F801060;
constexpr unsigned int MEMORY_CONTROL_2_END = MEMORY_CONTROL_2_START + MEMORY_CONTROL_2_SIZE;

bool MemoryControl::is_address_for_device(unsigned int address)
{
	if (address >= MEMORY_CONTROL_1_START && address <= MEMORY_CONTROL_1_END)
	{
		return true;
	}
	else if (address >= MEMORY_CONTROL_2_START && address <= MEMORY_CONTROL_2_END)
	{
		return true;
	}
	return false;
}

unsigned char MemoryControl::get_byte(unsigned int address)
{
	// TODO
	return 0;
}

void MemoryControl::set_byte(unsigned int address, unsigned char value)
{
	// TODO
}