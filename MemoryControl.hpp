#pragma once
#include "Bus.hpp"

class MemoryControl : public Bus::BusDevice
{
public:
	static MemoryControl * get_instance();

	virtual bool is_address_for_device(unsigned int address) final;

	virtual unsigned char get_byte(unsigned int address) final;

	virtual void set_byte(unsigned int address, unsigned char value) final;
private:

	MemoryControl() = default;
	~MemoryControl() = default;

	static const unsigned int MEMORY_CONTROL_1_SIZE = 36;
	static const unsigned int MEMORY_CONTROL_2_SIZE = 4;

	static const unsigned int MEMORY_CONTROL_1_START = 0x1F801000;
	static const unsigned int MEMORY_CONTROL_1_END = MEMORY_CONTROL_1_START + MEMORY_CONTROL_1_SIZE;

	static const unsigned int MEMORY_CONTROL_2_START = 0x1F801060;
	static const unsigned int MEMORY_CONTROL_2_END = MEMORY_CONTROL_2_START + MEMORY_CONTROL_2_SIZE;
};