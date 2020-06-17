#include <stdexcept>
#include <iostream>
#include <sstream>
#include <assert.h>
#include "Ram.hpp"
#include "SystemControlCoprocessor.hpp"

static Ram * instance = nullptr;

Ram * Ram::get_instance()
{
	if (instance == nullptr)
	{
		instance = new Ram();
	}

	return instance;
}

bool Ram::is_address_for_device(unsigned int address)
{
	if (address >= 0x0 && address < MAIN_MEMORY_SIZE)
	{
		return true;
	}
	else if (address >= 0x80000000 && address < 0x80000000 + MAIN_MEMORY_SIZE)
	{
		return true;
	}
	else if (address >= 0xa0000000 && address < 0xa0000000 + MAIN_MEMORY_SIZE)
	{
		return true;
	}
	else if (address >= 0x1f800000 && address < 0x1f800000 + SCRATCHPAD_SIZE)
	{
		return true;
	}
	else if (address >= 0x9f800000 && address < 0x9f800000 + SCRATCHPAD_SIZE)
	{
		return true;
	}

	return false;
}

unsigned char Ram::get_byte(unsigned int address)
{
	if ((address >= 0x0 && address < MAIN_MEMORY_SIZE) ||
		(address >= 0x80000000 && address < 0x80000000 + MAIN_MEMORY_SIZE) ||
		(address >= 0xa0000000 && address < 0xa0000000 + MAIN_MEMORY_SIZE))
	{
		system_control::status_register sr = SystemControlCoprocessor::get_instance()->get_control_register(system_control::register_names::SR);
		if (sr.Isc)
		{
			return scratchpad[address & 0x3FF];
		}
		else
		{
			return memory[address & 0x0FFFFFFF];
		}
	}
	else
	{
		return scratchpad[address & 0x3FF];
	}
}

void Ram::set_byte(unsigned int address, unsigned char value)
{
	if ((address >= 0x0 && address < MAIN_MEMORY_SIZE) ||
		(address >= 0x80000000 && address < 0x80000000 + MAIN_MEMORY_SIZE) ||
		(address >= 0xa0000000 && address < 0xa0000000 + MAIN_MEMORY_SIZE))
	{
		system_control::status_register sr = SystemControlCoprocessor::get_instance()->get_control_register(system_control::register_names::SR);
		if (sr.Isc)
		{
			scratchpad[address & 0x3FF] = value;
		}
		else
		{
			memory[address & 0x0FFFFFFF] = value;
		}
	}
	else
	{
		scratchpad[address & 0x3FF] = value;
	}
}

void Ram::save_state(std::stringstream& file)
{
	file.write(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
}

void Ram::load_state(std::stringstream& file)
{
	file.read(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
}

void Ram::reset()
{
	memset(memory, 0, MAIN_MEMORY_SIZE);
}