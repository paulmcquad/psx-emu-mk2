#pragma once
#include "Bus.hpp"

class CacheControl : public Bus::BusDevice
{
public:
	bool is_address_for_device(unsigned int address) final;

	unsigned char get_byte(unsigned int address) final;

	void set_byte(unsigned int address, unsigned char value) final;
};