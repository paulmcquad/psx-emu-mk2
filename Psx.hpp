#pragma once
#include <memory>
#include <string>
#include <sstream>

class Psx
{
public:
	bool init(std::string bios_path);
	bool load(std::string bin_path, std::string cue_path);
	void tick();
	void reset();

	void save_state(std::stringstream& state_stream);
	void load_state(std::stringstream& state_stream);
};