#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Exceptions.hpp"
#include "Cdrom.hpp"

// https://en.wikipedia.org/wiki/CD-ROM
// http://rveach.romhack.org/PSXInfo/psx%20hardware%20info.txt
// https://problemkaputt.de/psx-spx.htm#cdromdrive
constexpr unsigned int SECTOR_SIZE = 2352;
constexpr unsigned int STATUS_REGISTER = 0x1F801800 - CDROM_PORT_START;

// command registers
constexpr unsigned int COMMAND_REGISTER = 0x1F801801 - CDROM_PORT_START;
constexpr unsigned int PARAMETER_FIFO_REGISTER = 0x1F801802 - CDROM_PORT_START;
constexpr unsigned int REQUEST_REGISTER = 0x1F801803 - CDROM_PORT_START;

constexpr unsigned int INTERRUPT_FLAG_REGISTER = 0x1F801803 - CDROM_PORT_START;
constexpr unsigned int INTERRUPT_ENABLE_REGISTER_WRITE = 0x1F801802 - CDROM_PORT_START;
constexpr unsigned int INTERRUPT_ENABLE_REGISTER_READ = 0x1F801803 - CDROM_PORT_START;

constexpr unsigned int RESPONSE_FIFO_REGISTER = 0x1F801801 - CDROM_PORT_START;
constexpr unsigned int DATA_FIFO_REGISTER = 0x1F801802 - CDROM_PORT_START;

constexpr unsigned int RESPONSE_FIFO_SIZE = 16;
constexpr unsigned int PARAMETER_FIFO_SIZE = 16;
// double check
constexpr unsigned int DATA_FIFO_SIZE = 4096;

void Cdrom::init()
{
	response_fifo = new Fifo<unsigned char>(RESPONSE_FIFO_SIZE);
	data_fifo = new Fifo<unsigned char>(DATA_FIFO_SIZE);
	parameter_fifo = new Fifo<unsigned char>(PARAMETER_FIFO_SIZE);
}

Cdrom::~Cdrom()
{
	if (response_fifo)
	{
		delete response_fifo;
	}

	if (data_fifo)
	{
		delete data_fifo;
	}

	if (parameter_fifo)
	{
		delete parameter_fifo;
	}
}

void Cdrom::tick()
{
	if (response_interrupt_queue.empty() == false)
	{
		auto & top_response = response_interrupt_queue.front();
		if (top_response.first > 0 && top_response.second == current_response_received)
		{
			top_response.first--;
		}
	}
}

void Cdrom::trigger_pending_interrupts()
{
	// if interrupt queued and delay time has passed
	if (response_interrupt_queue.empty() == false)
	{
		auto & top_response = response_interrupt_queue.front();
		// delay passed and current response matches
		if (top_response.first == 0 && current_response_received == top_response.second)
		{
			response_interrupt_queue.pop_front();
			throw mips_interrupt();
		}
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
		status_register_read response;

		response.INDEX = register_index;
		response.ADPBUSY = false;
		response.PRMEMPT = parameter_fifo->is_empty();
		response.PRMWRDY = parameter_fifo->is_full() == false;
		response.RSLRRDY = response_fifo->is_empty() == false;
		response.DRQSTS = false;
		response.BUSYSTS = response_interrupt_queue.empty() == false;

		return response.raw;
	}
	else
	{
		switch (register_index)
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
		register_index = 0x03 & value;
	}
	else
	{
		switch (register_index)
		{
			case 0:
				return set_index0(address, value);
			case 1:
				return set_index1(address, value);
			default:
				throw std::logic_error("not implemented");
			}
	}
}

unsigned char Cdrom::get_index0(unsigned int address)
{
	switch (address)
	{
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
			interrupt_flag_register_read response = 0x0;
			response.response_received = static_cast<unsigned int>(current_response_received);
			// TODO more work is probably needed here

			return response.raw;
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
			parameter_fifo->push(value);
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

void Cdrom::set_index1(unsigned int address, unsigned char value)
{
	switch (address)
	{
		case INTERRUPT_FLAG_REGISTER:
		{
			interrupt_flag_register_write ack = value;

			// reset response
			current_response_received = 0;

			if (response_interrupt_queue.empty() == false)
			{
				unsigned int type = response_interrupt_queue.front().second;
				if (type == ack.ack_int1_7)
				{
					response_interrupt_queue.pop_front();
				}

				// if we have any more interrupts in the queue, set the next one as the current response
				if (response_interrupt_queue.empty() == false)
				{
					current_response_received = static_cast<unsigned int>(response_interrupt_queue.front().second);
				}
			}
			
			if (ack.reset_param_fifo)
			{
				parameter_fifo->clear();
			}

		} break;

		case INTERRUPT_ENABLE_REGISTER_WRITE:
		{
			interrupt_enable_register = value;
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_next_response_byte()
{
	unsigned char response_byte = 0x0;
	if (response_fifo->is_empty() == false)
	{
		response_byte = response_fifo->pop();
	}

	return response_byte;
}

unsigned char Cdrom::get_next_data_byte()
{
	unsigned char data_byte = 0x0;
	if (data_fifo->is_empty() == false)
	{
		data_byte = data_fifo->pop();
	}

	return data_byte;
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

		case cdrom_command::Getstat:
		{
			execute_getstat_command();
		} break;

		case cdrom_command::GetID:
		{
			execute_getid_command();
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
	unsigned char sub_function = parameter_fifo->pop();
	if (sub_function == 0x20)
	{
		// push the cd rom bios version onto the response fifo
		response_fifo->push(0x94);
		response_fifo->push(0x11);
		response_fifo->push(0x18);
		response_fifo->push(0xC0);

		current_response_received = static_cast<unsigned int>(cdrom_response_interrupts::FIRST_RESPONSE);
		unsigned int delay = static_cast<unsigned int>(cdrom_response_timings::FIRST_RESPONSE_DELAY);

		response_interrupt_queue.push_back(std::make_pair(delay, current_response_received));

		
	}
	else
	{
		throw std::logic_error("not implemented");
	}
}

void Cdrom::execute_getstat_command()
{
	// treated like a nop
	// 0x2 means the motor is on
	response_fifo->push(0x2);
	current_response_received = static_cast<unsigned int>(cdrom_response_interrupts::FIRST_RESPONSE);
	unsigned int delay = static_cast<unsigned int>(cdrom_response_timings::FIRST_RESPONSE_DELAY);

	response_interrupt_queue.push_back(std::make_pair(delay, current_response_received));
}

void Cdrom::execute_getid_command()
{
	execute_getstat_command();

	response_fifo->push(0x02);
	response_fifo->push(0x00);
	response_fifo->push(0x20);
	response_fifo->push(0x00);

	// report as licenced cd
	response_fifo->push(0x53); // S
	response_fifo->push(0x43); // C
	response_fifo->push(0x45); // E
	response_fifo->push(0x41); // A

	response_interrupt_queue.push_back(std::make_pair(static_cast<unsigned int>(cdrom_response_timings::SECOND_REPONSE_DELAY),
		static_cast<unsigned int>(cdrom_response_interrupts::SECOND_RESPONSE)));
}