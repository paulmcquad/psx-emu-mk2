#include "MemoryControl.hpp"

static MemoryControl * instance = nullptr;

MemoryControl * MemoryControl::get_instance()
{
	if (instance == nullptr)
	{
		instance = new MemoryControl();
	}

	return instance;
}

bool MemoryControl::is_address_for_device(unsigned int address)
{
	if (address >= MEMORY_CONTROL_1_START && address < MEMORY_CONTROL_1_END)
	{
		return true;
	}
	else if (address >= MEMORY_CONTROL_2_START && address < MEMORY_CONTROL_2_END)
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