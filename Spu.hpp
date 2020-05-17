#pragma once
#include "Dma.hpp"
#include "Bus.hpp"

constexpr unsigned int SPU_CONTROL_SIZE = 64;
constexpr unsigned int SPU_VOICE_SIZE = 576;

class Spu : public DMA_interface, public Bus::BusDevice
{
public:
	bool init();
	~Spu();
	bool is_address_for_device(unsigned int address) final;
	unsigned char get_byte(unsigned int address) final;
	void set_byte(unsigned int address, unsigned char value) final;
private:
	bool initialized = false;

	unsigned char spu_control[SPU_CONTROL_SIZE] = { 0 };
	unsigned char spu_voice_registers[SPU_VOICE_SIZE] = { 0 };
	unsigned char * spu_ram = nullptr;
};