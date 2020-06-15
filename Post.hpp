#pragma once
#include "Bus.hpp"

class Post : public Bus::BusDevice
{
public:
	static Post * get_instance();

	virtual bus_device_type get_bus_device_type() final;

	virtual bool is_address_for_device(unsigned int address) final;

	virtual void set_byte(unsigned int address, unsigned char value) final;

private:
	Post() = default;
	~Post() = default;
};