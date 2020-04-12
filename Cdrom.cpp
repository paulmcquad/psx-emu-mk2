#include <stdexcept>
#include "Cdrom.hpp"

void Cdrom::init()
{

}

unsigned char Cdrom::get(unsigned int address)
{
	throw std::logic_error("not implemented");
	return 0;
}

void Cdrom::set(unsigned int address, unsigned char value)
{
	throw std::logic_error("not implemented");
}