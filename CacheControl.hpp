#pragma once
#include "Bus.hpp"

class CacheControl : public Bus::BusDevice
{
public:
	virtual bus_device_type get_bus_device_type() final { return bus_device_type::CACHE_CONTROL;  }
	virtual bool is_address_for_device(unsigned int address) final;
	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;

private:
	static const unsigned int CACHE_CONTROL_SIZE = 512;
	static const unsigned int CACHE_CONTROL_START = 0xfffe0000;
	static const unsigned int CACHE_CONTROL_END = CACHE_CONTROL_START + CACHE_CONTROL_SIZE;
};