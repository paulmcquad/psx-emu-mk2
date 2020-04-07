#pragma once
#include <string>
#include <unordered_map>
#include <deque>
#include <iostream>

constexpr unsigned int MAIN_MEMORY_SIZE = 1024 * 512 * 4;
constexpr unsigned int PARALLEL_PORT_SIZE = 1024 * 64;
constexpr unsigned int SCRATCH_PAD_SIZE = 1024;
constexpr unsigned int IO_PORTS_SIZE = 1024 * 8;
constexpr unsigned int BIOS_SIZE = 1024 * 512;
constexpr unsigned int CACHE_CONTROL_SIZE = 512;

class IOPorts;

class Ram {
public:
	bool init(std::string bios_filepath, std::shared_ptr<IOPorts> _io_ports);
	void reset();
	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	void store_byte(unsigned int address, unsigned char value);
	void store_halfword(unsigned int address, unsigned short value);
	void store_word(unsigned int address, unsigned int value);

	unsigned char load_byte(unsigned int address);
	unsigned short load_halfword(unsigned int address);
	unsigned int load_word(unsigned int address);

private:
	void set_byte(unsigned int address, unsigned char value);
	unsigned char get_byte(unsigned int address);

	// four SRAM chips of 512KB
	unsigned char memory[MAIN_MEMORY_SIZE] = { 0 };
	unsigned char parallel_port[PARALLEL_PORT_SIZE] = { 0 };
	unsigned char scratch_pad[SCRATCH_PAD_SIZE] = { 0 };
	unsigned char bios[BIOS_SIZE] = { 0 };
	unsigned char cache_control[CACHE_CONTROL_SIZE] = { 0 };
	std::shared_ptr<IOPorts> io_ports;
};