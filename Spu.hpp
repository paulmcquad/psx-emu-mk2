#pragma once
#include "Dma.hpp"
#include "Bus.hpp"

class Spu : public DMA_interface, public Bus::BusDevice
{
public:
	bool init();
	~Spu();
	bool is_address_for_device(unsigned int address) final;
	unsigned char get_byte(unsigned int address) final;
	void set_byte(unsigned int address, unsigned char value) final;
	void reset();
private:
	static const unsigned int SPU_RAM_SIZE = 512 * 1042;
	static const unsigned int SPU_CONTROL_SIZE = 64;
	static const unsigned int SPU_VOICE_SIZE = 576;
	static const unsigned int SPU_START = 0x1F801C00;
	static const unsigned int SPU_END = SPU_START + SPU_VOICE_SIZE + SPU_CONTROL_SIZE;

	bool initialized = false;

	unsigned char spu_control[SPU_CONTROL_SIZE] = { 0 };
	unsigned char spu_voice_registers[SPU_VOICE_SIZE] = { 0 };
	unsigned char * spu_ram = nullptr;
};