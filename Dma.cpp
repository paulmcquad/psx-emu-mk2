#include "Dma.hpp"
#include <iostream>

constexpr unsigned int NUM_CHANNELS = 7;

constexpr unsigned int DMA_START = 0x1F801080;
constexpr unsigned int DMA_BASE_ADDRESS_START = 0x1F801080 - DMA_START;
constexpr unsigned int DMA_BLOCK_CONTROL_START = 0x1F801084 - DMA_START;
constexpr unsigned int DMA_CHANNEL_CONTROL_START = 0x1F801088 - DMA_START;
constexpr unsigned int DMA_CONTROL_REGISTER_START = 0x1F8010F0 - DMA_START;
constexpr unsigned int DMA_INTERRUPT_REGISTER_START = 0x1F8010F4 - DMA_START;
constexpr unsigned int DMA_GARBAGE_START = 0x1F8010F8 - DMA_START;

void Dma::init(std::shared_ptr<Ram> _ram, std::shared_ptr<Gpu> _gpu)
{
	ram = _ram;
	gpu = _gpu;

	for (int chan_idx = 0; chan_idx < NUM_CHANNELS; chan_idx++)
	{
		base_address_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[DMA_BASE_ADDRESS_START + (chan_idx*16)]);
		block_control_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[DMA_BLOCK_CONTROL_START + (chan_idx * 16)]);
		channel_control_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[DMA_CHANNEL_CONTROL_START + (chan_idx * 16)]);
	}

	control_register = reinterpret_cast<unsigned int*>(&dma_registers[DMA_CONTROL_REGISTER_START]);
	interrupt_register = reinterpret_cast<unsigned int*>(&dma_registers[DMA_INTERRUPT_REGISTER_START]);
	
	reset();
}

void Dma::reset()
{
	// reset control register
	
	*control_register = 0x07654321;
	*interrupt_register = 0x0;

	for (int chan_idx = 0; chan_idx < NUM_CHANNELS; chan_idx++)
	{
		*base_address_registers[chan_idx] = 0x0;
		*block_control_registers[chan_idx] = 0x0;
		*channel_control_registers[chan_idx] = 0x0;
	}
}

void Dma::tick()
{

}

unsigned char * Dma::operator[](unsigned int address)
{
	return &dma_registers[address];
}