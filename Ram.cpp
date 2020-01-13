#include "Ram.hpp"

constexpr unsigned int KUSEG_START    = 0x00000000;
constexpr unsigned int KUSEG_END      = 0x001fffff;

constexpr unsigned int KSEG0_START    = 0x80000000;
constexpr unsigned int KSEG0_END      = 0x801fffff;

constexpr unsigned int KSEG1_START    = 0xa0000000;
constexpr unsigned int KSEG1_END      = 0xa01fffff;

constexpr unsigned int PARALLEL_START = 0x1f000000;
constexpr unsigned int PARALLEL_END   = 0x1f00ffff;

constexpr unsigned int SCRATCH_START  = 0x1f800000;
constexpr unsigned int SCRATCH_END    = 0x1f8003ff;

constexpr unsigned int HARDWARE_START = 0x1f801000;
constexpr unsigned int HARDWARE_END   = 0x1f802fff;

constexpr unsigned int BIOS_START     = 0xbfc00000;
constexpr unsigned int BIOS_END       = 0xbfc7ffff;

unsigned char& Ram::operator[](unsigned int address) {
	// user memory mirroring
	if (address >= KUSEG_START && address <= KUSEG_END) {
		return memory[address];
	}
	else if (address >= KSEG0_START && address <= KSEG0_END) {
		return memory[address - KSEG0_START];
	}
	else if (address >= KSEG1_START && address <= KSEG1_END) {
		return memory[address - KSEG1_START];
	}
	else if (address >= PARALLEL_START && address <= PARALLEL_END) {
		return parallel_port[address - PARALLEL_START];
	}
	else if (address >= SCRATCH_START && address <= SCRATCH_END) {
		return scratch_pad[address - SCRATCH_START];
	}
	else if (address >= HARDWARE_START && address <= HARDWARE_END) {
		return hardware_registers[address - HARDWARE_START];
	}
	else if (address >= BIOS_START && address <= BIOS_END) {
		return bios[address - BIOS_START];
	}

	return unmapped_memory;
}