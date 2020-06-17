#pragma once
#include "Bus.hpp"

class ParallelPort : public Bus::BusDevice
{
public:
	static ParallelPort * get_instance();

	virtual bool is_address_for_device(unsigned int address) final;
	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;
private:

	ParallelPort() = default;
	~ParallelPort() = default;

	static const unsigned int PARALLEL_PORT_SIZE = 1024 * 64;
	static const unsigned int PARALLEL_START = 0x1f000000;
	static const unsigned int PARALLEL_END = PARALLEL_START + PARALLEL_PORT_SIZE;
};