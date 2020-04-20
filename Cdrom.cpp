#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Cdrom.hpp"

// https://en.wikipedia.org/wiki/CD-ROM
constexpr unsigned int SECTOR_SIZE = 2352;

constexpr unsigned int CDROM_PORT_START = 0x1F801800;

constexpr unsigned int STATUS_REGISTER = 0x1F801800 - CDROM_PORT_START;

// audio registers
constexpr unsigned int VOL_LEFT_CD_TO_LEFT_SPU_REGISTER = 0x1F801802 - CDROM_PORT_START;
constexpr unsigned int VOL_LEFT_CD_TO_RIGHT_SPU_REGISTER = 0x1F801803 - CDROM_PORT_START;
constexpr unsigned int VOL_RIGHT_CD_TO_RIGHT_SPU_REGISTER = 0x1F801801 - CDROM_PORT_START;
constexpr unsigned int VOL_RIGHT_CD_TO_LEFT_SPU_REGISTER = 0x1F801802 - CDROM_PORT_START;
constexpr unsigned int VOL_APPLY_CHANGES_REGISTER = 0x1F801803 - CDROM_PORT_START;
constexpr unsigned int SOUND_MAP_DATA_OUT_REGISTER = 0x1F801801 - CDROM_PORT_START;
constexpr unsigned int SOUND_MAP_CODING_INFO_REGISTER = 0x1F801801 - CDROM_PORT_START;

// command registers
constexpr unsigned int COMMAND_REGISTER = 0x1F801801 - CDROM_PORT_START;
constexpr unsigned int PARAMETER_FIFO_REGISTER = 0x1F801802 - CDROM_PORT_START;
constexpr unsigned int REQUEST_REGISTER = 0x1F801803 - CDROM_PORT_START;

constexpr unsigned int INTERRUPT_FLAG_REGISTER = 0x1F801803 - CDROM_PORT_START;
constexpr unsigned int INTERRUPT_ENABLE_REGISTER = 0x1F801802 - CDROM_PORT_START;

constexpr unsigned int RESPONSE_FIFO_REGISTER = 0x1F801801 - CDROM_PORT_START;

constexpr unsigned int DATA_FIFO_SIZE = 4096;
constexpr unsigned int RESPONSE_FIFO_SIZE = 16;

void Cdrom::init()
{
	data_fifo.data.resize(DATA_FIFO_SIZE);
	response_fifo.data.resize(RESPONSE_FIFO_SIZE);

	status_register.raw = 0x0;
	status_register.values.PRMEMPT = 1;

	interrupt_enable_register.raw = 0x0;
	interrupt_flag_response_register.raw = 0x0;
	interrupt_flag_response_register.values.response = 3;
}

void Cdrom::tick()
{

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
		int num_bytes = static_cast<int>(rom_file.tellg());
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
	if (address == STATUS_REGISTER)
	{
		return status_register.raw;
	}
	else
	{
		switch (status_register.values.INDEX)
		{
			case 0:
				return get_index0(address);
			case 1:
				return get_index1(address);
			case 2:
				return get_index2(address);
			case 3:
				return get_index3(address);
			default:
				throw std::logic_error("not implemented");
		}
	}
}

void Cdrom::set(unsigned int address, unsigned char value)
{
	if (address == STATUS_REGISTER)
	{
		// only the index value is writable
		status_register.values.INDEX = 0x03 & value;
	}
	else
	{
		switch (status_register.values.INDEX)
		{
			case 0:
				return set_index0(address, value);
			case 1:
				return set_index1(address, value);
			case 2:
				return set_index2(address, value);
			case 3:
				return set_index3(address, value);
			}
	}
}

unsigned char Cdrom::get_index0(unsigned int address)
{
	switch (address)
	{
		case INTERRUPT_ENABLE_REGISTER:
		{
			return interrupt_enable_register.raw;
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return response_fifo.get_next_byte();
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_index1(unsigned int address)
{
	switch (address)
	{
		case INTERRUPT_FLAG_REGISTER:
		{
			return interrupt_flag_response_register.raw;
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return response_fifo.get_next_byte();
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_index2(unsigned int address)
{
	switch (address)
	{
		case INTERRUPT_ENABLE_REGISTER:
		{
			return interrupt_enable_register.raw;
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return response_fifo.get_next_byte();
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_index3(unsigned int address)
{
	switch (address)
	{
		case INTERRUPT_FLAG_REGISTER:
		{
			return interrupt_flag_response_register.raw;
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return response_fifo.get_next_byte();
		} break;

	default:
		throw std::logic_error("not implemented");
	}
}

void Cdrom::set_index0(unsigned int address, unsigned char value)
{
	switch (address)
	{
		case COMMAND_REGISTER:
		{
			command_register = value;
		} break;

		case PARAMETER_FIFO_REGISTER:
		{
			parameter_fifo.set_next_byte(value);
		} break;

		case REQUEST_REGISTER:
		{
			throw std::logic_error("not implemented");
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

void Cdrom::set_index1(unsigned int address, unsigned char value)
{
	switch (address)
	{
		case SOUND_MAP_DATA_OUT_REGISTER:
		{
			sound_map_out = value;
		} break;

		case INTERRUPT_ENABLE_REGISTER:
		{
			interrupt_enable_register.raw = value;
		} break;

		case INTERRUPT_FLAG_REGISTER:
		{
			interrupt_flag_register.raw = value;
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

void Cdrom::set_index2(unsigned int address, unsigned char value)
{
	switch (address)
	{
		case VOL_LEFT_CD_TO_LEFT_SPU_REGISTER: {
			volume_left_cd_to_left_spu = value;
		}	break;

		case VOL_LEFT_CD_TO_RIGHT_SPU_REGISTER: {
			volume_left_cd_to_right_spu = value;
		} break;

		case SOUND_MAP_CODING_INFO_REGISTER: {
			sound_map_coding_info = value;
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

void Cdrom::set_index3(unsigned int address, unsigned char value)
{
	switch (address)
	{
		case VOL_RIGHT_CD_TO_RIGHT_SPU_REGISTER: {
			volume_right_cd_to_right_spu = value;
		}	break;

		case VOL_RIGHT_CD_TO_LEFT_SPU_REGISTER: {
			volume_right_cd_to_left_spu = value;
		} break;

		case VOL_APPLY_CHANGES_REGISTER: {
			volume_apply_changes = value;
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}