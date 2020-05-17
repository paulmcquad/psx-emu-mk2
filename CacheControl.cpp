#include "CacheControl.hpp"

constexpr unsigned int CACHE_CONTROL_SIZE = 512;
constexpr unsigned int CACHE_CONTROL_START = 0xfffe0000;
constexpr unsigned int CACHE_CONTROL_END = CACHE_CONTROL_START + CACHE_CONTROL_SIZE;

bool CacheControl::is_address_for_device(unsigned int address)
{
	if (address >= CACHE_CONTROL_START && address <= CACHE_CONTROL_END)
	{
		return true;
	}
	return false;
}

unsigned char CacheControl::get_byte(unsigned int address)
{
	// TODO
	return 0;
}

void CacheControl::set_byte(unsigned int address, unsigned char value)
{
	// TODO
}