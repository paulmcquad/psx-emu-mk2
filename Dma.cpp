#include "Dma.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include <iostream>
#include <fstream>

bool Dma::is_address_for_device(unsigned int address)
{
	if (address >= DMA_START && address < DMA_END)
	{
		return true;
	}
	return false;
}

unsigned char Dma::get_byte(unsigned int address)
{
	return dma_registers[address - DMA_START];
}

void Dma::set_byte(unsigned int address, unsigned char value)
{
	dma_registers[address - DMA_START] = value;
}

void Dma::init(std::shared_ptr<Bus> _bus, std::shared_ptr<Gpu> _gpu, std::shared_ptr<Spu> _spu)
{
	bus = _bus;

	for (int chan_idx = 0; chan_idx < NUM_CHANNELS; chan_idx++)
	{
		base_address_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[DMA_BASE_ADDRESS_START + (chan_idx*16)]);
		block_control_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[DMA_BLOCK_CONTROL_START + (chan_idx * 16)]);
		channel_control_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[DMA_CHANNEL_CONTROL_START + (chan_idx * 16)]);
	}

	control_register = reinterpret_cast<unsigned int*>(&dma_registers[DMA_CONTROL_REGISTER_START]);
	interrupt_register = reinterpret_cast<unsigned int*>(&dma_registers[DMA_INTERRUPT_REGISTER_START]);

	devices[static_cast<unsigned int>(DMA_channel_type::OTC)] = this;
	devices[static_cast<unsigned int>(DMA_channel_type::GPU)] = _gpu.get();
	devices[static_cast<unsigned int>(DMA_channel_type::SPU)] = _spu.get();
	
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
		base_address.int_value = *base_address_registers[chan_idx];

		DMA_block_control block_control;
		block_control.int_value = *block_control_registers[chan_idx];

		DMA_channel_control channel_control;
		channel_control.int_value = *channel_control_registers[chan_idx];

		DMA_channel_type type = static_cast<DMA_channel_type>(chan_idx);

		DMA_interface * device =  devices[static_cast<unsigned int>(type)];

		if (channel_control.start_busy == 1)
		{
			switch (static_cast<DMA_sync_mode>(channel_control.sync_mode))
			{
				case DMA_sync_mode::manual:
				{
					if (channel_control.start_trigger == 1)
					{
						channel_control.start_trigger = 0;
						device->sync_mode_manual(bus, base_address, block_control, channel_control);
						channel_control.start_busy = 0;
					}
				} break;

				case DMA_sync_mode::request:
				{
					channel_control.start_trigger = 0;
					device->sync_mode_request(bus, base_address, block_control, channel_control);
					channel_control.start_busy = 0;
				} break;

				case DMA_sync_mode::linked_list:
				{
					channel_control.start_trigger = 0;
					device->sync_mode_linked_list(bus, base_address, block_control, channel_control);
					channel_control.start_busy = 0;
				} break;
			}
		}

		*base_address_registers[chan_idx] = base_address.int_value;
		*block_control_registers[chan_idx] = block_control.int_value;
		*channel_control_registers[chan_idx] = channel_control.int_value;
	}
}

void Dma::save_state(std::stringstream& file)
{
	file.write(reinterpret_cast<char*>(&dma_registers[0]), sizeof(unsigned char) * 128);
}

void Dma::load_state(std::stringstream& file)
{
	file.read(reinterpret_cast<char*>(&dma_registers[0]), sizeof(unsigned char) * 128);
}

void Dma::sync_mode_manual(std::shared_ptr<Bus> bus, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	//std::cout << "Starting OTC manual DMA\n";
	unsigned int num_words = block_control.BC;

	DMA_direction dir = static_cast<DMA_direction>(channel_control.transfer_direction);
	DMA_address_step step = static_cast<DMA_address_step>(channel_control.memory_address_step);
	unsigned int addr = base_address.memory_address & 0x1ffffc;

	while (num_words > 0)
	{
		num_words--;
		if (num_words == 0)
		{
			bus->set_word(addr, 0xffffffff);
		}
		else
		{
			bus->set_word(addr, (addr - 4) & 0x1fffff);
		}

		addr += (step == DMA_address_step::increment ? 4 : -4);
	}
}