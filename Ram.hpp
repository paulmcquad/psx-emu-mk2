#pragma once
#include "Bus.hpp"

#include <string>
#include <unordered_map>
#include <deque>
#include <iostream>

class Ram : public Bus::BusDevice {
public:
	bool is_address_for_device(unsigned int address) final;

	unsigned char get_byte(unsigned int address) final;
	void set_byte(unsigned int address, unsigned char value) final;

	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

private:
	static const unsigned int MAIN_MEMORY_SIZE = 1024 * 512 * 4;
	// four SRAM chips of 512KB
	unsigned char memory[MAIN_MEMORY_SIZE] = { 0 };
};