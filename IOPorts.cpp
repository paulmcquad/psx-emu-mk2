#include "IOPorts.hpp"
#include <typeinfo.h>

void IOPorts::init()
{
	// memory control 1
	for (io_registers addr = io_registers::EXPANSION_1_BASE_ADDRESS;
		addr <= io_registers::COM_DELAY; /* no increment */) {
		io_map[addr] = (*this)[addr];
		addr = static_cast<io_registers>((static_cast<unsigned int>(addr) + 4));
	}

	// peripheral IO ports

	// memory control 2

	// interrupt control

	// dma registers

	// timers

	// CDROM registers

	// gpu registers

	// mdec registers

	// spu voice registers

	// spu control registers

	// spu reverb configuration area

	// spu internal registers

	// expansion region 2

	// expansion region 2 - dual serial port

	// expansion region 2 - int/dip/post

	// expansion region 3
}

unsigned char* IOPorts::operator[](unsigned int addr)
{
	return &data[addr];
}

unsigned char* IOPorts::operator[](io_registers io_reg)
{
	unsigned int base_address = static_cast<unsigned int>(io_registers::EXPANSION_1_BASE_ADDRESS);
	unsigned int address = static_cast<unsigned int>(io_reg) - base_address;
	return &data[address];
}