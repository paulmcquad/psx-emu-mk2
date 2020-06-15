#include "CacheControl.hpp"

static CacheControl * instance = nullptr;

CacheControl * CacheControl::get_instance()
{
	if (instance == nullptr)
	{
		instance = new CacheControl();
	}

	return instance;
}

bool CacheControl::is_address_for_device(unsigned int address)
{
	if (address >= CACHE_CONTROL_START && address < CACHE_CONTROL_END)
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