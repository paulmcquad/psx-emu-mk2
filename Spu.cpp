#include <stdexcept>
#include "Spu.hpp"

constexpr unsigned int SPU_START = 0x1F801C00;
constexpr unsigned int SPU_END = SPU_START + SPU_VOICE_SIZE + SPU_CONTROL_SIZE;

bool Spu::init()
{
	spu_ram = new unsigned char[512 * 1042];
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