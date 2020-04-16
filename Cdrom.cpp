#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Cdrom.hpp"

// https://en.wikipedia.org/wiki/CD-ROM
constexpr unsigned int SECTOR_SIZE = 2352;

constexpr unsigned int HEADER_SIZE = 16;
constexpr unsigned int MODE1_DATA_SIZE = 2048;
constexpr unsigned int MODE2_DATA_SIZE = 2336;

void Cdrom::init()
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
	std::cout << "CDROM " << std::endl;
	return 0;
}

void Cdrom::set(unsigned int address, unsigned char value)
{
	std::cout << "CDROM " << std::endl;
	//throw std::logic_error("not implemented");
}