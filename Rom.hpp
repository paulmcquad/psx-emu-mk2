#pragma once
#include "Bus.hpp"

constexpr unsigned int BIOS_SIZE = 1024 * 512;

class Rom : public Bus::BusDevice
{
public:
	bool is_address_for_device(unsigned int address) final;

	unsigned char get_byte(unsigned int address) final;

	bool load_bios(std::string bios_filepath);

private:
	unsigned char bios[BIOS_SIZE] = { 0 };
};