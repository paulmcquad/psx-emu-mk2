#pragma once
#include "Bus.hpp"

class Timers : public Bus::BusDevice
{
public:
	static Timers * get_instance();

	virtual bool is_address_for_device(unsigned int address) final;
	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;
private:
	Timers() = default;
	~Timers() = default;

	static const unsigned int TIMER_SIZE = 45;
	static const unsigned int TIMER_START = 0x1F801100;
	static const unsigned int TIMER_END = TIMER_START + TIMER_SIZE;
};