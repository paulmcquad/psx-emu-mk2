#include "Gpu.hpp"

void Gpu::init()
{
	// I have to allocate all the vram memory at runtime or
    // else I get a compiler out of heap space issue at compile time
	video_ram.resize(VRAM_SIZE, 0);
	GPUSTAT.ready_dma = true;
}

void Gpu::tick()
{

}