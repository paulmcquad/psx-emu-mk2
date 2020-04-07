#include <stdexcept>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "MemoryMap.hpp"
#include "IOPorts.hpp"
#include "Exceptions.hpp"

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

bool Ram::init(std::string bios_filepath, std::shared_ptr<IOPorts> _io_ports)
{
	if (bios_filepath.empty() == false)
	{
		std::ifstream bios_file(bios_filepath);

		if (bios_file.good())
		{
			bios_file.seekg(0, std::ios::end);
			std::streampos len = bios_file.tellg();

			bios_file.seekg(0, std::ios::beg);
			bios_file.read((char*)&bios, len);
			bios_file.close();
		}
		else
		{
			std::cerr << "Unable to load " << bios_filepath << "\n";
			return false;
		}
	}
	else
	{
		std::cerr << "No bios file specified!\n";
		return false;
	}

	io_ports = _io_ports;
	return true;
}

void Ram::reset()
{
	// todo
}

void Ram::save_state(std::ofstream& file)
{
	file.write(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
	file.write(reinterpret_cast<char*>(&parallel_port[0]), sizeof(unsigned char) * PARALLEL_PORT_SIZE);
	file.write(reinterpret_cast<char*>(&scratch_pad[0]), sizeof(unsigned char) * SCRATCH_PAD_SIZE);
	file.write(reinterpret_cast<char*>(&cache_control[0]), sizeof(unsigned char) * CACHE_CONTROL_SIZE);
}

void Ram::load_state(std::ifstream& file)
{
	file.read(reinterpret_cast<char*>(&memory[0]), sizeof(unsigned char) * MAIN_MEMORY_SIZE);
	file.read(reinterpret_cast<char*>(&parallel_port[0]), sizeof(unsigned char) * PARALLEL_PORT_SIZE);
	file.read(reinterpret_cast<char*>(&scratch_pad[0]), sizeof(unsigned char) * SCRATCH_PAD_SIZE);
	file.read(reinterpret_cast<char*>(&cache_control[0]), sizeof(unsigned char) * CACHE_CONTROL_SIZE);
}

void Ram::store_byte(unsigned int address, unsigned char value)
{
	set_byte(address, value);
}

void Ram::store_halfword(unsigned int address, unsigned short value)
{
	store_byte(address, value);
	value >>= 8;
	store_byte(address, value);
}

void Ram::store_word(unsigned int address, unsigned int value)
{
	store_byte(address, value);
	value >>= 8;
	store_byte(address + 1, value);
	value >>= 8;
	store_byte(address + 2, value);
	value >>= 8;
	store_byte(address + 3, value);
}

unsigned char Ram::load_byte(unsigned int address)
{
	return get_byte(address);
}

unsigned short Ram::load_halfword(unsigned int address)
{
	unsigned short result;
	result = load_byte(address + 1);
	result <<= 8;
	result |= load_byte(address);
	return result;
}

unsigned int Ram::load_word(unsigned int address)
{
	unsigned int result;
	result = load_byte(address + 3);
	result <<= 8;
	result |= load_byte(address + 2);
	result <<= 8;
	result |= load_byte(address + 1);
	result <<= 8;
	result |= load_byte(address);
	return result;
}

void Ram::set_byte(unsigned int address, unsigned char value)
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
	{
		if (address > KSEG2_START + 512)
		{
			throw bus_error();
		}
		cache_control[address - KSEG2_START] = value;
		return;
	} break;
	case memory_segment::KUSEG:
		segment_offset = KUSEG_START;
		break;
	}

	address -= segment_offset;
	if (address >= MAIN_MEMORY_START && address < MAIN_MEMORY_START + MAIN_MEMORY_SIZE) {
		memory[address] = value;
	}
	else if (address >= PARALLEL_START && address < PARALLEL_START + PARALLEL_PORT_SIZE) {
		parallel_port[address - PARALLEL_START] = value;
	}
	else if (address >= SCRATCH_START && address < SCRATCH_START + SCRATCH_PAD_SIZE) {
		scratch_pad[address - SCRATCH_START] = value;
	}
	else if (address >= IO_START && address < IO_START + IO_PORTS_SIZE) {
		io_ports->set(address - IO_START, value);
	}
	else if (address >= BIOS_START && address < BIOS_START + BIOS_SIZE) {
		bios[address - BIOS_START] = value;
	}
	else
	{
		throw bus_error();
	}
}

unsigned char Ram::get_byte(unsigned int address)
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
	{
		if (address > KSEG2_START + 512)
		{
			throw bus_error();
		}
		return cache_control[address - KSEG2_START];
	} break;
	case memory_segment::KUSEG:
		segment_offset = KUSEG_START;
		break;
	}

	address -= segment_offset;
	if (address >= MAIN_MEMORY_START && address < MAIN_MEMORY_START + MAIN_MEMORY_SIZE) {
		return memory[address];
	}
	else if (address >= PARALLEL_START && address < PARALLEL_START + PARALLEL_PORT_SIZE) {
		return parallel_port[address - PARALLEL_START];
	}
	else if (address >= SCRATCH_START && address < SCRATCH_START + SCRATCH_PAD_SIZE) {
		return scratch_pad[address - SCRATCH_START];
	}
	else if (address >= IO_START && address < IO_START + IO_PORTS_SIZE) {
		return io_ports->get(address - IO_START);
	}
	else if (address >= BIOS_START && address < BIOS_START + BIOS_SIZE) {
		return bios[address - BIOS_START];
	}

	throw bus_error();
}