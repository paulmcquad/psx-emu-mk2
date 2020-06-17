#include "Dma.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include "SystemControlCoprocessor.hpp"
#include <iostream>
#include <fstream>

static Dma * instance = nullptr;
Dma * Dma::get_instance()
{
	if (instance == nullptr)
	{
		instance = new Dma();
	}
	return instance;
}

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
	if (address >= DMA_INTERRUPT_REGISTER_START && address < DMA_INTERRUPT_REGISTER_START + 4)
	{
		return interrupt_register.byte_value[address - DMA_INTERRUPT_REGISTER_START];
	}
	return dma_registers[address - DMA_START];
}

void Dma::set_byte(unsigned int address, unsigned char value)
{
	if (address >= DMA_INTERRUPT_REGISTER_START && address < DMA_INTERRUPT_REGISTER_START + 4)
	{
		if (address >= DMA_INTERRUPT_REGISTER_START && address < DMA_INTERRUPT_REGISTER_START + 4)
		{
			// the uppermost byte is written to differently
			int byte_idx = address - DMA_INTERRUPT_REGISTER_START;
			if (byte_idx == 3)
			{
				// writing 1 to a bit resets it back to 0
				// let's say we wanted to ack irq 0
				// the value would be 0000001
				// let's say the current irq_flags value is 1110001 indicating an active irq 0
				// and a few other active irq bits which we want to preserver
				// if we invert the value to 1111110 and then & it against the flags it will replicate 
				// the correct behaviour
				// 
				interrupt_register.irq_flags &= ~value;
				// the upper bits of this value we ignore, as you cannot write to the IRQ master flag bit
			}
			else
			{
				interrupt_register.byte_value[byte_idx] = value;
			}
		}
	}
	dma_registers[address - DMA_START] = value;
}

void Dma::init()
{
	for (int chan_idx = 0; chan_idx < NUM_CHANNELS; chan_idx++)
	{
		base_address_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[(DMA_BASE_ADDRESS_START - DMA_START) + (chan_idx*16)]);
		block_control_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[(DMA_BLOCK_CONTROL_START - DMA_START) + (chan_idx * 16)]);
		channel_control_registers[chan_idx] = reinterpret_cast<unsigned int*>(&dma_registers[(DMA_CHANNEL_CONTROL_START - DMA_START) + (chan_idx * 16)]);
	}

	control_register = reinterpret_cast<unsigned int*>(&dma_registers[DMA_CONTROL_REGISTER_START - DMA_START]);

	devices[static_cast<unsigned int>(DMA_channel_type::OTC)] = this;
	devices[static_cast<unsigned int>(DMA_channel_type::GPU)] = Gpu::get_instance();
	devices[static_cast<unsigned int>(DMA_channel_type::SPU)] = Spu::get_instance();
	
	reset();
}

void Dma::reset()
{
	memset(dma_registers, 0, 128);

	// according to problem kaputt documentation
	*control_register = 0x07654321;
	interrupt_register.value = 0x0;
}

void Dma::tick()
{
	// this is used to compare against the DICR IRQ enable and IRQ flags fields
	unsigned char irq_mask = 0x1;
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

		bool dma_complete = false;
		if (channel_control.start_busy == 1)
		{
			switch (static_cast<DMA_sync_mode>(channel_control.sync_mode))
			{
				case DMA_sync_mode::manual:
				{
					if (channel_control.start_trigger == 1)
					{
						channel_control.start_trigger = 0;
						device->sync_mode_manual(base_address, block_control, channel_control);
						channel_control.start_busy = 0;
						dma_complete = true;
					}
				} break;

				case DMA_sync_mode::request:
				{
					channel_control.start_trigger = 0;
					device->sync_mode_request(base_address, block_control, channel_control);
					channel_control.start_busy = 0;
					dma_complete = true;
				} break;

				case DMA_sync_mode::linked_list:
				{
					channel_control.start_trigger = 0;
					device->sync_mode_linked_list(base_address, block_control, channel_control);
					channel_control.start_busy = 0;
					dma_complete = true;
				} break;
			}
		}

		*base_address_registers[chan_idx] = base_address.int_value;
		*block_control_registers[chan_idx] = block_control.int_value;
		*channel_control_registers[chan_idx] = channel_control.int_value;

		// according to problemkaputt, on dma completion IF enabled, the irq should be set
		if (dma_complete && (interrupt_register.irq_enable & irq_mask))
		{
			interrupt_register.irq_flags |= irq_mask;
		}

		// shift the mast
		// first channel it will be 0000001
		// second channel it will be 0000010 and so on
		irq_mask <<= 1;
	}

	// setup the interrupts, failing to setup this stuff
	// caused 1.5 months of stalled progress in this project
	bool previous_interrupt_master_flag_value = interrupt_register.irq_master_flag;

	bool irq_active = (interrupt_register.irq_enable & interrupt_register.irq_flags);
	if (interrupt_register.force_irq || (interrupt_register.irq_master_enable && irq_active))
	{
		interrupt_register.irq_master_flag = true;
	}
	else
	{
		interrupt_register.irq_master_flag = false;
	}

	if (previous_interrupt_master_flag_value == false && interrupt_register.irq_master_flag == true)
	{
		SystemControlCoprocessor::get_instance()->interrupt_status_register.IRQ3_DMA = true;
	}
}

void Dma::save_state(std::stringstream& file)
{
	file.write(reinterpret_cast<char*>(&dma_registers[0]), sizeof(unsigned char) * 128);
	file.write(reinterpret_cast<char*>(interrupt_register.byte_value), sizeof(unsigned char) * 4);
}

void Dma::load_state(std::stringstream& file)
{
	file.read(reinterpret_cast<char*>(&dma_registers[0]), sizeof(unsigned char) * 128);
	file.read(reinterpret_cast<char*>(interrupt_register.byte_value), sizeof(unsigned char) * 4);
}

void Dma::sync_mode_manual(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
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
			Bus::get_instance()->set_word(addr, 0xffffffff);
		}
		else
		{
			Bus::get_instance()->set_word(addr, (addr - 4) & 0x1fffff);
		}

		addr += (step == DMA_address_step::increment ? 4 : -4);
	}
}
