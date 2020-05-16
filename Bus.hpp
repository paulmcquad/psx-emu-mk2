#pragma once
#include <memory>

class Bus
{
public:
	class BusDevice
	{
	public:
		virtual bool is_address_for_device(unsigned int address) = 0;

		virtual unsigned char get_byte(unsigned int address) = 0;
		virtual unsigned short get_halfword(unsigned int address) = 0;
		virtual unsigned int get_word(unsigned int address) = 0;

		virtual void set_byte(unsigned int address, unsigned char value) = 0;
		virtual void set_halfword(unsigned int address, unsigned short value) = 0;
		virtual void set_word(unsigned int address, unsigned int value) = 0;
	};

	void register_device(BusDevice * device);

	unsigned char get_byte(unsigned int address);
	unsigned short get_halfword(unsigned int address);
	unsigned int get_word(unsigned int address);

	void set_byte(unsigned int address, unsigned char value);
	void set_halfword(unsigned int address, unsigned short value);
	void set_word(unsigned int address, unsigned int value);

private:

	BusDevice * get_bus_device_for_address(unsigned int address);

	int num_devices = 0;
	BusDevice * bus_devices[7] = { nullptr };
};