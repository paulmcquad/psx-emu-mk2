#include "Rom.hpp"
#include <fstream>
#include <iostream>
#include <string>

static Rom * instance = nullptr;

Rom * Rom::get_instance()
{
	if (instance == nullptr)
	{
		instance = new Rom();
	}

	return instance;
}

bool Rom::is_address_for_device(unsigned int address)
{
	if (address >= 0x1FC00000 && address < 0x1FC00000 + BIOS_SIZE)
	{
		return true;
	}
	else if (address >= 0x9FC00000 && address < 0x9FC00000 + BIOS_SIZE)
	{
		return true;
	}
	else if (address >= 0xBFC00000 && address < 0xBFC00000 + BIOS_SIZE)
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