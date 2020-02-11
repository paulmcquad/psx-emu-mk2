#include "Gpu.hpp"

void Gpu::init()
{
	// I have to allocate all the vram memory at runtime or
    // else I get a compiler out of heap space issue at compile time
	video_ram.resize(VRAM_SIZE, 128);
	GPUSTAT.ready_dma = true;
}

void Gpu::tick()
{

	if (GPUSTAT.display_enable == false)
	{
#ifndef _DEBUG
		draw_static();
#endif
	}
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