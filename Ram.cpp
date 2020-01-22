#include "Ram.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <assert.h>

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

constexpr unsigned int CACHE_CONTROL_START = 0xfffe0000;
constexpr unsigned int CACHE_CONTROL_END = 0xfffe0200;

void Ram::init(std::string bios_filepath)
{
	std::ifstream bios_file(bios_filepath);
	bios_file.seekg(0, std::ios::end);
	unsigned int len = bios_file.tellg();

	assert(len == 1024 * 512);

	bios_file.seekg(0, std::ios::beg);
	bios_file.read((char*)&bios, len);
	bios_file.close();
}

void Ram::tick()
{
	if (store_queue.empty()) {
		return;
	}

	// 1 opcode delay to store instructions
	for (auto& iter : store_queue)
	{
		iter.tick_wait--;
	}

	store_entry& front = store_queue.front();
	if (front.tick_wait == 0)
	{
		unsigned char * location = get_byte(front.addr);
		switch (front.type) {
			case store_entry::store_type::type_byte:
			{
				*location = front.byte_value;
			} break;

			case store_entry::store_type::type_halfword:
			{
				*reinterpret_cast<unsigned short*>(location) = front.halfword_value;
			} break;

			case store_entry::store_type::type_word:
			{
				*reinterpret_cast<unsigned int*>(location) = front.word_value;
			} break;
		}
		store_queue.pop_front();
	}
}

unsigned char Ram::load_byte(unsigned int address)
{
	return *get_byte(address);
}

unsigned short Ram::load_halfword(unsigned int address)
{
	return *reinterpret_cast<unsigned short*>(get_byte(address));
}

unsigned int Ram::load_word(unsigned int address)
{
	return *reinterpret_cast<unsigned int*>(get_byte(address));
}

void Ram::store_byte(unsigned int address, unsigned char value)
{
	if (address % 4 != 0)
	{
		throw std::logic_error("unaligned access");
	}

	store_entry entry;
	entry.type = store_entry::store_type::type_byte;
	entry.byte_value = value;
	entry.addr = address;
	store_queue.push_back(entry);
}

void Ram::store_halfword(unsigned int address, unsigned short value)
{
	if (address % 4 != 0)
	{
		throw std::logic_error("unaligned access");
	}

	store_entry entry;
	entry.type = store_entry::store_type::type_halfword;
	entry.halfword_value = value;
	entry.addr = address;
	store_queue.push_back(entry);
}

void Ram::store_word(unsigned int address, unsigned int value)
{
	if (address % 4 != 0)
	{
		throw std::logic_error("unaligned access");
	}

	store_entry entry;
	entry.type = store_entry::store_type::type_word;
	entry.word_value = value;
	entry.addr = address;
	store_queue.push_back(entry);
}

unsigned char* Ram::get_byte(unsigned int address)
{
	// user memory mirroring
	if (address >= KUSEG_START && address <= KUSEG_END) {
		return &memory[address];
	}
	else if (address >= KSEG0_START && address <= KSEG0_END) {
		return &memory[address - KSEG0_START];
	}
	else if (address >= KSEG1_START && address <= KSEG1_END) {
		return &memory[address - KSEG1_START];
	}
	else if (address >= PARALLEL_START && address <= PARALLEL_END) {
		return &parallel_port[address - PARALLEL_START];
	}
	else if (address >= SCRATCH_START && address <= SCRATCH_END) {
		return &scratch_pad[address - SCRATCH_START];
	}
	else if (address >= HARDWARE_START && address <= HARDWARE_END) {
		return &hardware_registers[address - HARDWARE_START];
	}
	else if (address >= BIOS_START && address <= BIOS_END) {
		return &bios[address - BIOS_START];
	}
	else if (address >= CACHE_CONTROL_START && address <= CACHE_CONTROL_END) {
		return &cache_control[address - CACHE_CONTROL_START];
	}

	return nullptr;
}