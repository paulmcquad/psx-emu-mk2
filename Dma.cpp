#include "Dma.hpp"
#include "MemoryMap.hpp"
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
		// TODO make sure these are being set right - base register is wrong in tests
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
	for (int chan_idx = 0; chan_idx < NUM_CHANNELS; chan_idx++)
	{
		DMA_base_address base_address;
		base_address.int_value = *channel_control_registers[chan_idx];

		DMA_block_control block_control;
		block_control.int_value = *block_control_registers[chan_idx];

		DMA_channel_control channel_control;
		channel_control.int_value = *channel_control_registers[chan_idx];

		if (channel_control.start_busy == 1)
		{
			switch (static_cast<sync_mode>(channel_control.sync_mode))
			{
			case sync_mode::manual:
			{
				sync_mode_manual(chan_idx, base_address, block_control, channel_control);
			} break;

			case sync_mode::request:
			{
				sync_mode_request(chan_idx, base_address, block_control, channel_control);
			} break;

			case sync_mode::linked_list:
			{
				// doesn't use base address
				sync_mode_linked_list(chan_idx, block_control, channel_control);
			} break;
			}
		}

		*base_address_registers[chan_idx] = base_address.int_value;
		*block_control_registers[chan_idx] = block_control.int_value;
		*channel_control_registers[chan_idx] = channel_control.int_value;
	}
}

void Dma::sync_mode_manual(unsigned int channel, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	if (channel_control.start_trigger == 1)
	{
		channel_control.start_trigger = 0;

		unsigned int num_words = block_control.BC;

		direction dir = static_cast<direction>(channel_control.transfer_direction);
		address_step step = static_cast<address_step>(channel_control.memory_address_step);
		channel_type type = static_cast<channel_type>(channel);
		unsigned int addr = base_address.memory_address & 0x1ffffc;

		while (num_words > 0)
		{
			num_words--;
			// can only go to_ram if OTC
			if (type == channel_type::OTC)
			{
				if (num_words == 0)
				{
					ram->store<unsigned int>(addr, 0xffffffff);
				}
				else
				{
					ram->store<unsigned int>(addr, (addr - 4) & 0x1fffff);
				}
			}
			else
			{
				throw std::logic_error("not implemented");
			}
			
			addr += (step == address_step::increment ? 4 : -4);
		}

		channel_control.start_busy = 0;
	}
}

void Dma::sync_mode_request(unsigned int channel, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	channel_control.start_trigger = 0;

	channel_type type = static_cast<channel_type>(channel);
	throw std::logic_error("not implemented");
	
	channel_control.start_busy = 0;
}

void Dma::sync_mode_linked_list(unsigned int channel, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	channel_control.start_trigger = 0;

	channel_type type = static_cast<channel_type>(channel);
	throw std::logic_error("not implemented");

	channel_control.start_busy = 0;
}

unsigned char Dma::get(unsigned int address)
{
	return dma_registers[address];
}

void Dma::set(unsigned int address, unsigned char value)
{
	dma_registers[address] = value;
}