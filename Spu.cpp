#include <stdexcept>
#include "Spu.hpp"

void Spu::init()
{

}

unsigned char Spu::get(unsigned int address)
{
	throw std::logic_error("not implemented");
	return 0;
}

void Spu::set(unsigned int address, unsigned char value)
{
	throw std::logic_error("not implemented");
}