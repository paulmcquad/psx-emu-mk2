#pragma once
#include "Bus.hpp"

#include <string>
#include <unordered_map>
#include <deque>
#include <iostream>

class Ram : public Bus::BusDevice {
public:
	static Ram * get_instance();

	virtual bus_device_type get_bus_device_type() final { return bus_device_type::RAM; }

	virtual bool is_address_for_device(unsigned int address) final;
	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;

	void save_state(std::stringstream& file);
	void load_state(std::stringstream& file);
	void reset();

private:
	Ram() = default;
	~Ram() = default;

	static const unsigned int MAIN_MEMORY_SIZE = 1024 * 512 * 4;
	// four SRAM chips of 512KB
	unsigned char memory[MAIN_MEMORY_SIZE] = { 0 };
};