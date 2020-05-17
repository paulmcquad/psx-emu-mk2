#include "ParallelPort.hpp"

constexpr unsigned int PARALLEL_PORT_SIZE = 1024 * 64;
constexpr unsigned int PARALLEL_START = 0x1f000000;
constexpr unsigned int PARALLEL_END = PARALLEL_START + PARALLEL_PORT_SIZE;

bool ParallelPort::is_address_for_device(unsigned int address)
{
	if (address >= PARALLEL_START && address <= PARALLEL_END)
	{
		return true;
	}
	return false;
}

unsigned char ParallelPort::get_byte(unsigned int address)
{
	// TODO
	return 0;
}

void ParallelPort::set_byte(unsigned int address, unsigned char value)
{
	// TODO
}