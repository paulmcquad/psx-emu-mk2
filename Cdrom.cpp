#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Cdrom.hpp"

// https://en.wikipedia.org/wiki/CD-ROM
constexpr unsigned int SECTOR_SIZE = 2352;

constexpr unsigned int CDROM_PORT_START = 0x1F801800;

constexpr unsigned int STATUS_REGISTER_START = 0x1F801800 - CDROM_PORT_START;
constexpr unsigned int STATUS_REGISTER_END = 0x1F801801 - CDROM_PORT_START;



void Cdrom::init()
{
	status_register.raw = 0x0;

	interrupt_flag_register.raw = 0x0;
	interrupt_flag_register.values.na4 = 0x1;
	interrupt_flag_register.values.na3 = 0x1;
	interrupt_flag_register.values.na2 = 0x1;
	interrupt_flag_register.values.na1 = 0x0;

	interrupt_enable_register.raw = 0x0;
}

void Cdrom::save_state(std::ofstream& file)
{

}

void Cdrom::load_state(std::ifstream& file)
{

}

bool Cdrom::load(std::string bin_file, std::string /*cue_file*/)
{
	std::ifstream rom_file(bin_file, std::ios::binary);

	if (rom_file.is_open())
	{
		rom_file.seekg(0, rom_file.end);
		int num_bytes = rom_file.tellg();
		rom_file.seekg(0, rom_file.beg);

		rom_data.resize(num_bytes);

		rom_file.read((char*)&rom_data[0], num_bytes);

		num_sectors = num_bytes / SECTOR_SIZE;

		std::cout << "Loaded: " << bin_file << " Num Sectors: " << num_sectors << "\n";

		rom_file.close();

		return true;
	}

	return false;
}

unsigned char Cdrom::get(unsigned int address)
{
	if (address >= STATUS_REGISTER_START &&
		address < STATUS_REGISTER_END)
	{

		return status_register.raw;
	}
	// depending on the index, different io ports operate differently
	else if (status_register.values.INDEX == 0)
	{

	}
	else if (status_register.values.INDEX == 1)
	{

	}
	else if (status_register.values.INDEX == 2)
	{

	}
	else
	{

	}

	return 0;
}

void Cdrom::set(unsigned int address, unsigned char value)
{
	if (address >= STATUS_REGISTER_START &&
		address < STATUS_REGISTER_END)
	{
		// only the index value is writable
		status_register.values.INDEX = 0x03 & value;
	}
	// depending on the index, different io ports operate differently
	else if (status_register.values.INDEX == 0)
	{

	}
	else if (status_register.values.INDEX == 1)
	{

	}
	else if (status_register.values.INDEX == 2)
	{

	}
	else
	{

	}
}