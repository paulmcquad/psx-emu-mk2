#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Cdrom.hpp"
#include "DebugMenuManager.hpp"

static Cdrom * instance = nullptr;

Cdrom * Cdrom::get_instance()
{
	if (instance == nullptr)
	{
		instance = new Cdrom();
	}

	return instance;
}

bool Cdrom::is_address_for_device(unsigned int address)
{
	if (address >= CDROM_START && address < CDROM_END)
	{
		return true;
	}
	return false;
}

unsigned char Cdrom::get_byte(unsigned int address)
{
	try
	{
		return get(address);
	}
	catch (...)
	{
		std::cerr << "Error get: " << register_index << "-" << std::hex << address << std::endl;
		return 0;
	}
}

void Cdrom::set_byte(unsigned int address, unsigned char value)
{
	try
	{
		set(address, value);
	}
	catch (...)
	{
		std::cerr << "Error set: " << register_index << "-" << std::hex << address << std::endl;
	}	
}

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
	if (interrupt_enable_register)
	{
		if (current_int == cdrom_response_interrupts::NO_RESPONSE)
		{
			if (pending_response.empty() == false)
			{
				pending_response_data data = pending_response.front();
				pending_response.pop_front();
				current_int = data.int_type;
				time_to_irq = data.delay;
				interrupt_countdown_active = true;
				for (auto & iter : data.responses)
				{
					response_fifo->push(iter);
				}
			}
		}

		if (interrupt_countdown_active)
		{
			time_to_irq--;
			if (time_to_irq <= 0)
			{
				SystemControlCoprocessor::get_instance()->set_irq_bits(system_control::CDROM_BIT);
				interrupt_countdown_active = false;
			}
		}
	}
}

void Cdrom::reset()
{
	response_fifo->clear();
	data_fifo->clear();
	parameter_fifo->clear();

	register_index = 0;
	current_int = cdrom_response_interrupts::NO_RESPONSE;

	interrupt_enable_register = 0x0;
}

void Cdrom::save_state(std::stringstream& file)
{
	{
		std::vector<unsigned int> responses;
		while (response_fifo->get_current_size() > 0)
		{
			responses.push_back(response_fifo->pop());
		}
		unsigned int num_responses = responses.size();
		file.write(reinterpret_cast<char*>(&num_responses), sizeof(unsigned int));
		file.write(reinterpret_cast<char*>(responses.data()), sizeof(unsigned int)*num_responses);
	}
	
	{
		std::vector<unsigned int> data;
		while (data_fifo->get_current_size() > 0)
		{
			data.push_back(data_fifo->pop());
		}
		unsigned int num_data = data.size();
		file.write(reinterpret_cast<char*>(&num_data), sizeof(unsigned int));
		file.write(reinterpret_cast<char*>(data.data()), sizeof(unsigned int)*num_data);
	}

	{
		std::vector<unsigned int> params;
		while (parameter_fifo->get_current_size() > 0)
		{
			params.push_back(parameter_fifo->pop());
		}
		unsigned int num_params = params.size();
		file.write(reinterpret_cast<char*>(&num_params), sizeof(unsigned int));
		file.write(reinterpret_cast<char*>(params.data()), sizeof(unsigned int)*num_params);
	}

	file.write(reinterpret_cast<char*>(&register_index), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&current_int), sizeof(cdrom_response_interrupts));
	file.write(reinterpret_cast<char*>(&interrupt_enable_register), sizeof(unsigned int));
}

void Cdrom::load_state(std::stringstream& file)
{
	{
		unsigned int num_responses = 0;
		file.read(reinterpret_cast<char*>(&num_responses), sizeof(unsigned int));

		std::vector<unsigned int> responses;
		responses.resize(num_responses);
		file.read(reinterpret_cast<char*>(responses.data()), sizeof(unsigned int)*num_responses);

		response_fifo->clear();

		for (auto iter : responses)
		{
			response_fifo->push(iter);
		}
	}

	{
		unsigned int num_data = 0;
		file.read(reinterpret_cast<char*>(&num_data), sizeof(unsigned int));

		std::vector<unsigned int> data;
		data.resize(num_data);
		file.read(reinterpret_cast<char*>(data.data()), sizeof(unsigned int)*num_data);

		data_fifo->clear();

		for (auto iter : data)
		{
			data_fifo->push(iter);
		}
	}

	{
		unsigned int num_params = 0;
		file.read(reinterpret_cast<char*>(&num_params), sizeof(unsigned int));

		std::vector<unsigned int> params;
		params.resize(num_params);
		file.read(reinterpret_cast<char*>(params.data()), sizeof(unsigned int)*num_params);

		parameter_fifo->clear();

		for (auto iter : params)
		{
			parameter_fifo->push(iter);
		}
	}

	file.read(reinterpret_cast<char*>(&register_index), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&current_int), sizeof(cdrom_response_interrupts));
	file.read(reinterpret_cast<char*>(&interrupt_enable_register), sizeof(unsigned int));
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
	if (address == 0x1F801800)
	{
		status_register status;
		status.INDEX = register_index;
		status.ADPBUSY = false; // todo
		status.PRMEMPT = parameter_fifo->is_empty();
		status.PRMWRDY = parameter_fifo->is_full() == false;
		status.RSLRRDY = response_fifo->is_empty() == false;
		status.DRQSTS = data_fifo->is_empty() == false;
		status.BUSYSTS = pending_response.empty() == false;

		//DebugMenuManager::get_instance()->paused_requested = true;

		return status.raw;
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
	if (address == 0x1F801800)
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
		case 0x1F801801:
		{
			return get_next_response_byte();
		} break;

		case 0x1F801802:
		{
			return get_next_data_byte();
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_index1(unsigned int address)
{
	switch (address)
	{
		case 0x1F801801:
		{
			return get_next_response_byte();
		} break;

		case 0x1F801802:
		{
			return get_next_data_byte();
		} break;

		case 0x1F801803:
		{
			interrupt_flag_register_read irq_reg;
			irq_reg.response_received = static_cast<unsigned int>(current_int);
			return irq_reg.raw;
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_index2(unsigned int address)
{
	switch (address)
	{
		case 0x1F801801:
		{
			return get_next_response_byte();
		} break;

		case 0x1F801802:
		{
			return get_next_data_byte();
		} break;

		default:
			throw std::logic_error("not implemented");
	}
}

unsigned char Cdrom::get_index3(unsigned int address)
{
	switch (address)
	{
		case 0x1F801801:
		{
			return get_next_response_byte();
		} break;

		case 0x1F801802:
		{
			return get_next_data_byte();
		} break;

	default:
		throw std::logic_error("not implemented");
	}
}

void Cdrom::set_index0(unsigned int address, unsigned char value)
{
	switch (address)
	{
		case 0x1F801801:
		{
			execute_command(value);
		} break;

		case 0x1F801802:
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
		case 0x1f801802:
		{
			interrupt_enable_register = value;
		} break;

		case 0x1f801803:
		{
			interrupt_flag_register_write irq_rg = value;

			if (irq_rg.ack_int1_7)
			{
				interrupt_countdown_active = false;
				current_int = cdrom_response_interrupts::NO_RESPONSE;
			}
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

		case cdrom_command::ReadTOC:
		{
			execute_read_toc_command();
		} break;

		case cdrom_command::Setloc:
		{
			execute_set_loc_command();
		} break;

		case cdrom_command::SeekL:
		{
			execute_seek_l_command();
		} break;

		case cdrom_command::Setmode:
		{
			execute_set_mode_command();
		} break;

		case cdrom_command::ReadN:
		{
			execute_read_n_command();
		}

		default:
			std::cerr << "Command: " << std::hex << static_cast<unsigned int>(command) << std::endl;
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
		pending_response_data data;
		data.delay = cdrom_response_timings::FIRST_RESPONSE_DELAY;
		data.int_type = cdrom_response_interrupts::FIRST_RESPONSE;

		// push the cd rom bios version onto the response fifo
		data.responses.push_back(0xC0);
		data.responses.push_back(0x18);
		data.responses.push_back(0x11);
		data.responses.push_back(0x94);

		pending_response.push_back(data);
	}
	else
	{
		throw std::logic_error("not implemented");
	}
}

void Cdrom::execute_getstat_command()
{
	pending_response_data data;
	data.delay = cdrom_response_timings::FIRST_RESPONSE_DELAY;
	data.int_type = cdrom_response_interrupts::FIRST_RESPONSE;
	data.responses.push_back(0x2);
	pending_response.push_back(data);
}

void Cdrom::execute_getid_command()
{
	execute_getstat_command();

	pending_response_data data;
	data.int_type = cdrom_response_interrupts::SECOND_RESPONSE;
	data.delay = cdrom_response_timings::SECOND_REPONSE_DELAY;

	data.responses.push_back(0x02); // stat
	data.responses.push_back(0x00); // flags

	data.responses.push_back(0x20); // type
	data.responses.push_back(0x00); // atip

	data.responses.push_back(0x53); // S
	data.responses.push_back(0x43); // C
	data.responses.push_back(0x45); // E
	data.responses.push_back(0x41); // A
	pending_response.push_back(data);

}

// this command seems a bit pointless
void Cdrom::execute_read_toc_command()
{
	execute_getstat_command();

	pending_response_data data;
	data.int_type = cdrom_response_interrupts::SECOND_RESPONSE;
	data.delay = cdrom_response_timings::SECOND_REPONSE_DELAY;
	data.responses.push_back(0x02);
	pending_response.push_back(data);
}

void Cdrom::execute_set_loc_command()
{
	execute_getstat_command();

	seek_target.amm = parameter_fifo->pop();
	seek_target.ass = parameter_fifo->pop();
	seek_target.asect = parameter_fifo->pop();
}

// this command actually sets the location, the set_loc input
void Cdrom::execute_seek_l_command()
{
	location = seek_target;

	execute_getstat_command();

	pending_response_data data;
	data.int_type = cdrom_response_interrupts::SECOND_RESPONSE;
	data.delay = cdrom_response_timings::SECOND_REPONSE_DELAY;
	data.responses.push_back(0x02);
	pending_response.push_back(data);
}

void Cdrom::execute_set_mode_command()
{
	mode.raw = parameter_fifo->pop();

	execute_getstat_command();
}

void Cdrom::execute_read_n_command()
{
	// todo
}