#include "IOPorts.hpp"
#include "Gpu.hpp"
#include "Exceptions.hpp"
#include <typeinfo.h>

constexpr unsigned int IO_START = 0x1f801000;
constexpr unsigned int GP0_Send = 0x1f801810 - IO_START;
constexpr unsigned int GP1_Send = 0x1f801814 - IO_START;
constexpr unsigned int GPUREAD = 0x1f801810 - IO_START;
constexpr unsigned int GPUSTAT = 0x1f801814 - IO_START;

void IOPorts::init(std::shared_ptr<Gpu> _gpu)
{
	gpu = _gpu;
}

unsigned char* IOPorts::get(unsigned int address, bool read)
{
	if (read)
	{
		return read_access(address);
	}
	else
	{
		return write_access(address);
	}
}

unsigned char* IOPorts::read_access(unsigned int addr)
{
	switch (addr)
	{
		case GPUREAD:
		{
			return gpu->GPUREAD.byte_value;
		} break;

		case GPUSTAT:
		{
			return gpu->GPUSTAT.byte_value;
		}

		default:
		{
			throw bus_error();
		}
	}
}

unsigned char* IOPorts::write_access(unsigned int addr)
{
	switch (addr)
	{
		case GP0_Send:
		{
			return gpu->GP0_send.byte_value;
		} break;

		case GP1_Send:
		{
			return gpu->GP1_send.byte_value;
		} break;

		default:
		{
			throw bus_error();
		}
	}
}