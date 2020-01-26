#include "MemoryMap.hpp"
#include "IOPorts.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <assert.h>

enum class memory_segment : unsigned char
{
	KUSEG = 0b000,
	KSEG0 = 0b100,
	KSEG1 = 0b101,
	KSEG2 = 0b111
};

constexpr unsigned int KUSEG_START    = 0x00000000;
constexpr unsigned int KSEG0_START    = 0x80000000;
constexpr unsigned int KSEG1_START    = 0xa0000000;
constexpr unsigned int KSEG2_START    = 0xfffe0000;

constexpr unsigned int MAIN_MEMORY_START = 0x00000000;
constexpr unsigned int PARALLEL_START = 0x1f000000;
constexpr unsigned int SCRATCH_START  = 0x1f800000;
constexpr unsigned int IO_START = 0x1f801000;
constexpr unsigned int BIOS_START     = 0x1fc00000;

void Ram::init(std::string bios_filepath, std::shared_ptr<IOPorts> _io_ports)
{
	std::ifstream bios_file(bios_filepath);
	bios_file.seekg(0, std::ios::end);
	unsigned int len = bios_file.tellg();

	assert(len == 1024 * 512);

	bios_file.seekg(0, std::ios::beg);
	bios_file.read((char*)&bios, len);
	bios_file.close();

	io_ports = _io_ports;
}

unsigned char Ram::load_byte(unsigned int address)
{
	return *get_byte(address);
}

unsigned short Ram::load_halfword(unsigned int address)
{
	if (address % 2 != 0)
	{
		throw std::logic_error("unaligned access");
	}

	unsigned char * loc = get_byte(address);
	return *reinterpret_cast<unsigned short*>(loc);
}

unsigned int Ram::load_word(unsigned int address)
{
	if (address % 4 != 0)
	{
		throw std::logic_error("unaligned access");
	}

	unsigned char * loc = get_byte(address);
	return *reinterpret_cast<unsigned int*>(loc);
}

void Ram::store_byte(unsigned int address, unsigned char value)
{
	unsigned char * loc = get_byte(address);
	if (loc)
	{
		*get_byte(address) = value;
	}
	else
	{
		throw std::logic_error("out of bounds access");
	}
	
}

void Ram::store_halfword(unsigned int address, unsigned short value)
{
	if (address % 2 != 0)
	{
		throw std::logic_error("unaligned access");
	}

	unsigned char * loc = get_byte(address);
	if (loc)
	{
		*reinterpret_cast<unsigned short*>(loc) = value;
	}
	else
	{
		throw std::logic_error("out of bounds access");
	}
}

void Ram::store_word(unsigned int address, unsigned int value)
{
	if (address % 4 != 0)
	{
		throw std::logic_error("unaligned access");
	}

	unsigned char * loc = get_byte(address);

	if (loc)
	{
		*reinterpret_cast<unsigned int*>(loc) = value;
	}
	else
	{
		throw std::logic_error("out of bounds access");
	}
}

unsigned char* Ram::get_byte(unsigned int address)
{
	// determine memory region
	unsigned char segment_value = address >> 29;
	memory_segment segment = memory_segment::KUSEG;
	if (segment_value > 0b011)
	{
		segment = static_cast<memory_segment>(segment_value);
	}

	unsigned int segment_offset = 0;
	switch (segment) {
		case memory_segment::KSEG0:
			segment_offset = KSEG0_START;
			break;
		case memory_segment::KSEG1:
			segment_offset = KSEG1_START;
			break;
		case memory_segment::KSEG2:
			return &cache_control[address - KSEG2_START];
			break;
		case memory_segment::KUSEG:
			segment_offset = KUSEG_START;
			break;
	}

	address -= segment_offset;
	if (address >= MAIN_MEMORY_START && address < MAIN_MEMORY_START + MAIN_MEMORY_SIZE) {
		return &memory[address];
	}
	else if (address >= PARALLEL_START && address < PARALLEL_START + PARALLEL_PORT_SIZE) {
		return &parallel_port[address - PARALLEL_START];
	}
	else if (address >= SCRATCH_START && address < SCRATCH_START + SCRATCH_PAD_SIZE) {
		return &scratch_pad[address - SCRATCH_START];
	}
	else if (address >= IO_START && address < IO_START + IO_PORTS_SIZE) {
		return (*io_ports)[address - IO_START];
	}
	else if (address >= BIOS_START && address < BIOS_START + BIOS_SIZE) {
		return &bios[address - BIOS_START];
	}

	return nullptr;
}