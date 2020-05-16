#include "Bus.hpp"
#include <stdexcept>

void Bus::register_device(Bus::BusDevice * device)
{
	bus_devices[num_devices] = device;
	num_devices++;
}

unsigned char Bus::get_byte(unsigned int address)
{
	BusDevice * device = get_bus_device_for_address(address);
	if (device)
	{
		return device->get_byte(address);
	}

	throw std::logic_error("bus device not implemented!");
	return 0;
}

unsigned short Bus::get_halfword(unsigned int address)
{
	BusDevice * device = get_bus_device_for_address(address);
	if (device)
	{
		return device->get_halfword(address);
	}

	throw std::logic_error("bus device not implemented!");
	return 0;
}

unsigned int Bus::get_word(unsigned int address)
{
	BusDevice * device = get_bus_device_for_address(address);
	if (device)
	{
		return device->get_word(address);
	}

	throw std::logic_error("bus device not implemented!");
	return 0;
}

void Bus::set_byte(unsigned int address, unsigned char value)
{
	BusDevice * device = get_bus_device_for_address(address);
	if (device)
	{
		device->set_byte(address, value);
		return;
	}

	throw std::logic_error("bus device not implemented!");
}

void Bus::set_halfword(unsigned int address, unsigned short value)
{
	BusDevice * device = get_bus_device_for_address(address);
	if (device)
	{
		device->set_halfword(address, value);
		return;
	}

	throw std::logic_error("bus device not implemented!");
}

void Bus::set_word(unsigned int address, unsigned int value)
{
	BusDevice * device = get_bus_device_for_address(address);
	if (device)
	{
		device->set_word(address, value);
		return;
	}

	throw std::logic_error("bus device not implemented!");
}

Bus::BusDevice * Bus::get_bus_device_for_address(unsigned int address)
{
	for (int idx = 0; idx < num_devices; idx++)
	{
		BusDevice * device = bus_devices[idx];
		if (device->is_address_for_device(address))
		{
			return device;
		}
	}
	return nullptr;
}