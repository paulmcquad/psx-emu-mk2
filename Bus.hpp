#pragma once
#include <memory>
#include <stdexcept>

class Bus
{
public:
	class BusDevice
	{
	public:
		enum class bus_device_type {GPU, SPU, RAM, ROM, CDROM, PARALLEL_PORT, MEMORY_CONTROL, TIMERS, DMA, CACHE_CONTROL, INTERRUPT_CONTROL, UNKNOWN};

		virtual bus_device_type get_bus_device_type() = 0;

		virtual bool is_peripheral() {
			return false;
		}

		virtual bool is_address_for_device(unsigned int address) = 0;

		virtual unsigned char get_byte(unsigned int address)
		{
			throw std::logic_error("not implemented");
			return 0;
		}

		virtual unsigned short get_halfword(unsigned int address)
		{
			unsigned short result = 0;

			result = (get_byte(address + 1) << 8) |
				get_byte(address);

			return result;
		}

		virtual unsigned int get_word(unsigned int address)
		{
			unsigned int result = 0;

			result = (get_byte(address + 3) << 24) |
				(get_byte(address + 2) << 16) |
				(get_byte(address + 1) << 8) |
				get_byte(address);

			return result;
		}

		virtual void set_byte(unsigned int address, unsigned char value)
		{
			throw std::logic_error("not implemented");
		}

		virtual void set_halfword(unsigned int address, unsigned short value)
		{
			for (int offset = 0; offset < 2; offset++)
			{
				unsigned int current_address = address + offset;
				unsigned char byte_value = value & 0xFF;
				value >>= 8;
				set_byte(current_address, byte_value);
			}
		}

		virtual void set_word(unsigned int address, unsigned int value)
		{
			for (int offset = 0; offset < 4; offset++)
			{
				unsigned int current_address = address + offset;
				unsigned char byte_value = value & 0xFF;
				value >>= 8;
				set_byte(current_address, byte_value);
			}
		}
	};

	void register_device(BusDevice * device);

	unsigned char get_byte(unsigned int address);
	unsigned short get_halfword(unsigned int address);
	unsigned int get_word(unsigned int address);

	void set_byte(unsigned int address, unsigned char value);
	void set_halfword(unsigned int address, unsigned short value);
	void set_word(unsigned int address, unsigned int value);

	bool is_address_to_ignore(unsigned int address);

	bool suppress_exceptions = false;

	unsigned int address_to_pause_on = 0x0;
	bool enable_pause_on_address_access = false;
	bool request_pause = false;

	// debug only
	bool currently_accessing_peripheral = false;
	BusDevice::bus_device_type device_being_accessed = BusDevice::bus_device_type::UNKNOWN;

private:

	BusDevice * get_bus_device_for_address(unsigned int address);

	int num_devices = 0;
	BusDevice * bus_devices[20] = { nullptr };
};