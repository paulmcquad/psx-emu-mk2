#include "Dma.hpp"

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

	base_addresses.resize(NUM_CHANNELS);
	block_controls.resize(NUM_CHANNELS);
	channel_controls.resize(NUM_CHANNELS); 

	for (int channel_idx = 0; channel_idx < NUM_CHANNELS; channel_idx++)
	{
		unsigned int register_offset = channel_idx * 0x10;

		for (int byte_offset = 0; byte_offset < 4; byte_offset++)
		{
			{
				unsigned char * byte_ref = &base_addresses[channel_idx].byte_value[byte_offset];
				unsigned int addr = (DMA_BASE_ADDRESS_START + register_offset) + byte_offset;
				raw_ref_buffer[addr] = byte_ref;
			}

			{
				unsigned char * byte_ref = &block_controls[channel_idx].byte_value[byte_offset];
				unsigned int addr = (DMA_BLOCK_CONTROL_START + register_offset) + byte_offset;
				raw_ref_buffer[addr] = byte_ref;
			}

			{
				unsigned char * byte_ref = &channel_controls[channel_idx].byte_value[byte_offset];
				unsigned int addr = (DMA_CHANNEL_CONTROL_START + register_offset) + byte_offset;
				raw_ref_buffer[addr] = byte_ref;
			}
		}
	}

	for (int byte_offset = 0; byte_offset < 4; byte_offset++)
	{
		{
			unsigned char * byte_ref = &control_register.byte_value[byte_offset];
			unsigned int addr = DMA_CONTROL_REGISTER_START + byte_offset;
			raw_ref_buffer[addr] = byte_ref;
		}

		{
			unsigned char * byte_ref = &interrupt_register.byte_value[byte_offset];
			unsigned int addr = DMA_INTERRUPT_REGISTER_START + byte_offset;
			raw_ref_buffer[addr] = byte_ref;
		}

		{
			unsigned char * byte_ref = &garbage[byte_offset];
			unsigned int addr = DMA_GARBAGE_START + byte_offset;
			raw_ref_buffer[addr] = byte_ref;
		}

		{
			unsigned char * byte_ref = &garbage[byte_offset + 4];
			unsigned int addr = DMA_GARBAGE_START + byte_offset + 4;
			raw_ref_buffer[addr] = byte_ref;
		}
	}
}

unsigned char * Dma::operator[](unsigned int address)
{
	return raw_ref_buffer[address];
}