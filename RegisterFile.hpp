#include <iostream>
#include <fstream>

class RegisterFile
{
public:
	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	void reset();
	void tick();

	unsigned int get_register(unsigned int index, bool ignore_load_delay = false);
	void set_register(unsigned int index, unsigned int value, bool load_delay = false);

private:
	unsigned int stage_1_registers[32] = { 0 };
	unsigned int stage_2_registers[32] = { 0 };
	unsigned int stage_3_registers[32] = { 0 };
};