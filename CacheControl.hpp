#pragma once
#include "Bus.hpp"

class CacheControl : public Bus::BusDevice
{
public:
	static CacheControl * get_instance();

	virtual bool is_address_for_device(unsigned int address) final;

	virtual unsigned int get_word(unsigned int address) final;
	virtual void set_word(unsigned int address, unsigned int value) final;

	union
	{
		unsigned int raw;
		struct
		{
			unsigned int na0 : 3;
			unsigned int scratch_pad_enable1 : 1;
			unsigned int na1 : 2;
			unsigned int na2 : 1;
			unsigned int scratch_pad_enable2 : 1;
			unsigned int na3 : 1;
			unsigned int crash : 1;
			unsigned int na4 : 1;
			unsigned int code_cache_enable : 1;
			unsigned int na5 : 20;
		};
	} cache_control_register;

private:
	CacheControl() = default;
	~CacheControl() = default;

	static const unsigned int CACHE_CONTROL_SIZE = 4;
	static const unsigned int CACHE_CONTROL_START = 0xFFFE0130;
	static const unsigned int CACHE_CONTROL_END = CACHE_CONTROL_START + CACHE_CONTROL_SIZE;
};