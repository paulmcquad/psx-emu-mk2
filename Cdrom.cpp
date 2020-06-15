#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Cdrom.hpp"

static Cdrom * instance = nullptr;

Cdrom * Cdrom::get_instance()
{
	if (instance == nullptr)
	{
		instance = new Cdrom();
	}

	return instance;
}

bool Cdrom::trigger_pending_interrupts(SystemControlCoprocessor* system_control_processor, unsigned int & excode)
{
	// interrupt active and no unacknowledged interrupts
	if (system_control_processor->interrupt_mask_register.IRQ2_CDROM == true &&
		system_control_processor->interrupt_status_register.IRQ2_CDROM == false &&
		awaiting_acknowledgement == false)
	{
		// if interrupt queued and delay time has passed
		if (response_interrupt_queue.empty() == false)
		{
			auto & top_response = response_interrupt_queue.front();
			// delay passed and current response matches
			if (top_response.first == 0)
			{
				response_interrupt_queue.pop_front();
				system_control_processor->interrupt_mask_register.IRQ2_CDROM = true;
				excode = static_cast<unsigned int>(system_control::excode::INT);
				awaiting_acknowledgement = true;
				return true;
			}
		}
	}
	return false;
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
	return get(address);
}

void Cdrom::set_byte(unsigned int address, unsigned char value)
{
	set(address, value);
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
	if (response_interrupt_queue.empty() == false && awaiting_acknowledgement == false)
	{
		auto & top_response = response_interrupt_queue.front();
		if (top_response.first > 0)
		{
			top_response.first--;
		}
	}
}

void Cdrom::reset()
{
	response_fifo->clear();
	data_fifo->clear();
	parameter_fifo->clear();
	response_interrupt_queue.clear();

	register_index = 0;
	current_response_received = 0;
	awaiting_acknowledgement = false;

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
	file.write(reinterpret_cast<char*>(&current_response_received), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&interrupt_enable_register), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&awaiting_acknowledgement), sizeof(bool));
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
	file.read(reinterpret_cast<char*>(&current_response_received), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&interrupt_enable_register), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&awaiting_acknowledgement), sizeof(bool));
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
		response.ADPBUSY = false; // not implemented yet
		response.PRMEMPT = parameter_fifo->is_empty();
		response.PRMWRDY = parameter_fifo->is_full() == false;
		response.RSLRRDY = response_fifo->is_empty() == false;
		response.DRQSTS = data_fifo->is_empty() == false;
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
			awaiting_acknowledgement = false;

			if (response_interrupt_queue.empty() == false)
			{
				unsigned int type = response_interrupt_queue.front().second;
				response_interrupt_queue.pop_front();

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

		case cdrom_command::ReadTOC:
		{
			execute_read_toc_command();
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

// this command seems a bit pointless
void Cdrom::execute_read_toc_command()
{
	execute_getstat_command();
	response_fifo->push(0x02);
	response_interrupt_queue.push_back(std::make_pair(static_cast<unsigned int>(cdrom_response_timings::SECOND_REPONSE_DELAY),
		static_cast<unsigned int>(cdrom_response_interrupts::SECOND_RESPONSE)));
}