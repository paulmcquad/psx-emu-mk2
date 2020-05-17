#include "Rom.hpp"
#include <fstream>
#include <iostream>
#include <string>

constexpr unsigned int KUSEG_START = 0x1FC00000;
constexpr unsigned int KSEG0_START = 0x9FC00000;
constexpr unsigned int KSEG1_START = 0xBFC00000;

bool Rom::is_address_for_device(unsigned int address)
{
	if (address >= KUSEG_START && address <= KUSEG_START + BIOS_SIZE)
	{
		return true;
	}
	else if (address >= KSEG0_START && address <= KSEG0_START + BIOS_SIZE)
	{
		return true;
	}
	else if (address >= KSEG1_START && address <= KSEG1_START + BIOS_SIZE)
	{
		return true;
	}

	return false;
}

unsigned char Rom::get_byte(unsigned int address)
{
	return bios[address & 0x000FFFFF];
}

bool Rom::load_bios(std::string bios_filepath)
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
	return true;
}