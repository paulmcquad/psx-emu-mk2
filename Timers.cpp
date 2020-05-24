#include "Timers.hpp"

bool Timers::is_address_for_device(unsigned int address)
{
	if (address >= TIMER_START && address < TIMER_END)
	{
		return true;
	}
	return false;
}

unsigned char Timers::get_byte(unsigned int address)
{
	// TODO
	return 0;
}
void Timers::set_byte(unsigned int address, unsigned char value)
{
	// TODO
}