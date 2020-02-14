#include "Gpu.hpp"
#include "MemoryMap.hpp"
#include <iostream>

void Gpu::init()
{
	// I have to allocate all the vram memory at runtime or
    // else I get a compiler out of heap space issue at compile time
	video_ram.resize(VRAM_SIZE, 128);
	// hardcoded according to simias guide to get the emulator moving a bit further through the code
	GPUSTAT.ready_dma = true;
	GPUSTAT.ready_cmd_word = true;
}

void Gpu::tick()
{

	if (GPUSTAT.display_enable == false)
	{
#ifndef _DEBUG
		//draw_static();
#endif
	}
}

void Gpu::sync_mode_request(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{

	std::cout << "Starting GPU request DMA\n";
	unsigned int num_words = block_control.BS * block_control.BC;
	DMA_address_step step = static_cast<DMA_address_step>(channel_control.memory_address_step);
	unsigned int addr = base_address.memory_address & 0x1ffffc;

	while (num_words > 0)
	{
		num_words--;

		unsigned int word = ram->load<unsigned int>(addr);

		// TODO do something with this word

		addr += (step == DMA_address_step::increment ? 4 : -4);
	}
	std::cout << "Finished GPU request DMA\n";
}

void Gpu::sync_mode_linked_list(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	std::cout << "Starting GPU linked list DMA\n";
	unsigned int addr = base_address.memory_address & 0x1ffffc;
	while (true)
	{
		unsigned int header = ram->load<unsigned int>(addr);
		unsigned int num_words = header >> 24;

		while (num_words > 0)
		{
			addr = (addr + 4) & 0x1ffffc;

			unsigned int command = ram->load<unsigned int>(addr);

			// TODO do something with this command

			num_words--;
		}

		if ((header & 0x800000) != 0)
		{
			break;
		}
		else
		{
			addr = header & 0x1ffffc;
		}
	}
	std::cout << "Finished GPU linked list DMA\n";
}

void Gpu::draw_static()
{
	for (int height = 0; height < FRAME_HEIGHT; height++)
	{
		for (int width = 0; width < FRAME_WIDTH; width++)
		{
			unsigned int index = height * FRAME_WIDTH + width;
			index *= BYTES_PER_PIXEL;

			unsigned char value = rand() % 255;

			video_ram[index] = value;
			video_ram[index + 1] = value;
			video_ram[index + 2] = value;
		}
	}
}