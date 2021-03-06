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

unsigned int CacheControl::get_word(unsigned int address)
{
	return cache_control_register.raw;
}

void CacheControl::set_word(unsigned int address, unsigned int value)
{
	cache_control_register.raw = value;
}