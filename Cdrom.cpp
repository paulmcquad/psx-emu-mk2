#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Exceptions.hpp"
#include "Cdrom.hpp"

// https://en.wikipedia.org/wiki/CD-ROM
// http://rveach.romhack.org/PSXInfo/psx%20hardware%20info.txt
// https://problemkaputt.de/psx-spx.htm#cdromdrive
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
constexpr unsigned int INTERRUPT_ENABLE_REGISTER_WRITE = 0x1F801802 - CDROM_PORT_START;
constexpr unsigned int INTERRUPT_ENABLE_REGISTER_READ = 0x1F801803 - CDROM_PORT_START;

constexpr unsigned int RESPONSE_FIFO_REGISTER = 0x1F801801 - CDROM_PORT_START;
constexpr unsigned int DATA_FIFO_REGISTER = 0x1F801802 - CDROM_PORT_START;

void Cdrom::init()
{
	status_register.raw = 0x0;
	status_register.PRMEMPT = 1;

	interrupt_enable_register.raw = 0x0;
	interrupt_flag_response_register.raw = 0x0;
	interrupt_flag_response_register.response = 3;
}

void Cdrom::tick()
{

}

void Cdrom::trigger_pending_interrupts()
{
	if (response_interrupt_queue.empty() == false)
	{
		interrupt_flag_register.ack_1_7 = static_cast<unsigned char>(response_interrupt_queue.front());
		response_interrupt_queue.pop_front();

		throw mips_interrupt();
	}
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
		switch (status_register.INDEX)
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
		status_register.INDEX = 0x03 & value;
	}
	else
	{
		switch (status_register.INDEX)
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
		case INTERRUPT_ENABLE_REGISTER_READ:
		{
			return interrupt_enable_register.raw;
		} break;

		case DATA_FIFO_REGISTER:
		{
			return get_next_data_byte();
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return get_next_response_byte();
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

		case DATA_FIFO_REGISTER:
		{
			return get_next_data_byte();
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return get_next_response_byte();
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_index2(unsigned int address)
{
	switch (address)
	{
		case INTERRUPT_ENABLE_REGISTER_READ:
		{
			return interrupt_enable_register.raw;
		} break;

		case DATA_FIFO_REGISTER:
		{
			return get_next_data_byte();
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return get_next_response_byte();
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

		case DATA_FIFO_REGISTER:
		{
			return get_next_data_byte();
		} break;

		case RESPONSE_FIFO_REGISTER:
		{
			return get_next_response_byte();
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
			execute_command(value);
		} break;

		case PARAMETER_FIFO_REGISTER:
		{
			parameter_fifo.push_back(value);
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

		case INTERRUPT_ENABLE_REGISTER_WRITE:
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

unsigned char Cdrom::get_next_response_byte()
{
	unsigned char response_byte = 0x0;
	if (response_fifo.empty() == false)
	{
		response_byte = response_fifo.front();
		response_fifo.pop_front();

		status_register.RSLRRDY = (response_fifo.empty() == false);
	}
	return response_byte;
}

unsigned char Cdrom::get_next_data_byte()
{
	// todo
	return 0;
}

void Cdrom::execute_command(unsigned char command)
{
	cdrom_command command_enum = static_cast<cdrom_command>(command);

	switch (command_enum)
	{
		case cdrom_command::Test:
		{
			execute_test_command();
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

void Cdrom::execute_test_command()
{
	// the test to run is determined by the subfunction on the parameter fifo
	// however, the ps1 only uses 0x20 which returns the cd rom bios version
	// so no need to implement anything but that
	unsigned char sub_function = parameter_fifo.front();
	if (sub_function == 0x20)
	{
		// discard parameters from the fifo
		parameter_fifo.pop_front();

		// push the cd rom bios version onto the response fifo
		response_fifo.push_back(0x94);
		response_fifo.push_back(0x09);
		response_fifo.push_back(0x19);
		response_fifo.push_back(0xC0);

		response_interrupt_queue.push_back(cdrom_response_interrupts::FIRST_RESPONSE);
	}
	else
	{
		throw std::logic_error("not implemented");
	}
}