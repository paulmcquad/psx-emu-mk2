#pragma once
#include "Bus.hpp"

class Rom : public Bus::BusDevice
{
public:
	static Rom * get_instance();

	virtual bool is_address_for_device(unsigned int address) final;
	virtual unsigned char get_byte(unsigned int address) final;

	bool load_bios(std::string bios_filepath);

private:
	Rom() = default;
	~Rom() = default;

	static const unsigned int BIOS_SIZE = 1024 * 512;

	unsigned char bios[BIOS_SIZE] = { 0 };
};