#include <stdexcept>
#include "Spu.hpp"

bool Spu::init()
{
	spu_ram = new unsigned char[SPU_RAM_SIZE];
	return true;
}

Spu::~Spu()
{
	if (spu_ram)
	{
		delete[] spu_ram;
	}
}

bool Spu::is_address_for_device(unsigned int address)
{
	if (address >= SPU_START && address <= SPU_END)
	{
		return true;
	}
	return false;
}

unsigned char Spu::get_byte(unsigned int address)
{
	//throw std::logic_error("not implemented");
	return 0;
}

void Spu::set_byte(unsigned int address, unsigned char value)
{
	//throw std::logic_error("not implemented");
}