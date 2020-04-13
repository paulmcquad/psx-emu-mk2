#pragma once
#include "Dma.hpp"

constexpr unsigned int SPU_CONTROL_SIZE = 64;
constexpr unsigned int SPU_VOICE_SIZE = 576;

class Spu : public DMA_interface
{
public:
	bool init();
	~Spu();
	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);
private:
	bool initialized = false;

	unsigned char spu_control[SPU_CONTROL_SIZE] = { 0 };
	unsigned char spu_voice_registers[SPU_VOICE_SIZE] = { 0 };
	unsigned char * spu_ram = nullptr;
};