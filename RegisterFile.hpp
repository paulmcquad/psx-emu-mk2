#pragma once
#include <iostream>
#include <sstream>

class RegisterFile
{
public:
	void save_state(std::stringstream& file);
	void load_state(std::stringstream& file);

	void reset();
	void tick();

	unsigned int get_register(unsigned int index, bool ignore_load_delay = false);
	void set_register(unsigned int index, unsigned int value, bool load_delay = false);

	// used for debug purposes only
	unsigned int * get_register_ref(unsigned int index)
	{
		return &stage_3_registers[index];
	}
	bool register_just_changed = false;
	int index_of_register_changed = 0;
	unsigned int value_of_register_changed = 0;
	bool break_on_change[32] = { false };
	bool register_just_overwritten = false;
	bool break_on_overwrite = false;
	bool only_pause_on_value = false;
	int pause_on_value = 0x0;

	// debug use only
	// count when values in load delay get erased
	unsigned int overwrites[32] = { 0 };

private:
	unsigned int stage_1_registers[32] = { 0 };
	unsigned int stage_2_registers[32] = { 0 };
	unsigned int stage_3_registers[32] = { 0 };
};