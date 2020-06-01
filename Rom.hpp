#pragma once
#include "Bus.hpp"

class Rom : public Bus::BusDevice
{
public:
	virtual bus_device_type get_bus_device_type() final { return bus_device_type::ROM; }
	virtual bool is_address_for_device(unsigned int address) final;
	virtual unsigned char get_byte(unsigned int address) final;

	bool load_bios(std::string bios_filepath);

private:
	static const unsigned int BIOS_SIZE = 1024 * 512;

	unsigned char bios[BIOS_SIZE] = { 0 };
};