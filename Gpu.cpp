#include "Gpu.hpp"
#include "MemoryMap.hpp"
#include "InstructionEnums.hpp"
#include <iostream>
#include <algorithm>

union vert_command
{
	unsigned int value;
	struct
	{
		int x : 11;
		unsigned int na0 : 5;
		int y : 11;
		unsigned int na1 : 5;
	};
};

union color_command
{
	unsigned int value;
	struct
	{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char op;
	};
};

void Gpu::init()
{
	// I have to allocate all the vram memory at runtime or
    // else I get a compiler out of heap space issue at compile time
	video_ram.resize(VRAM_SIZE, 0x0);
	// hardcoded according to simias guide to get the emulator moving a bit further through the code
	GPUSTAT.ready_dma = true;
	GPUSTAT.ready_cmd_word = true;

	gp0_command_map[gp0_commands::NOP] = &Gpu::nop;
	gp0_command_map[gp0_commands::SET_DRAW_TOP_LEFT] = &Gpu::set_draw_top_left;
	gp0_command_map[gp0_commands::SET_DRAW_BOTTOM_RIGHT] = &Gpu::set_draw_bottom_right;
	gp0_command_map[gp0_commands::SET_DRAWING_OFFSET] = &Gpu::set_drawing_offset;
	gp0_command_map[gp0_commands::DRAW_MODE] = &Gpu::set_draw_mode;
	gp0_command_map[gp0_commands::TEX_WINDOW] = &Gpu::set_texture_window;
	gp0_command_map[gp0_commands::MASK_BIT] = &Gpu::set_mask_bit;
	gp0_command_map[gp0_commands::MONO_4_PT_OPAQUE] = &Gpu::mono_4_pt_opaque;
}

void Gpu::reset()
{
	
}

void Gpu::tick()
{
	while (command_queue.empty() == false)
	{
		color_command command;
		command.value = command_queue.front();

		gp0_commands current_command = static_cast<gp0_commands>(command.op);

		// let's assume it always finds the function ptr :)
		auto iter = gp0_command_map.find(current_command);
		if (iter != gp0_command_map.end())
		{
			unsigned int commands_to_remove = (this->*iter->second)();
			if (commands_to_remove == 0)
			{
				// waiting for more words
				break;
			}
			while (commands_to_remove > 0)
			{
				command_queue.pop_front();
				commands_to_remove--;
			}
		}
		else
		{
			std::cout << std::hex << command.value << std::endl;
			throw std::logic_error("not implemented");
		}
	}
}

void Gpu::sync_mode_request(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{

	std::cout << "Starting GPU request DMA\n";
	unsigned int num_words = block_control.BS * block_control.BC;
	DMA_address_step step = static_cast<DMA_address_step>(channel_control.memory_address_step);
	unsigned int addr = base_address.memory_address & 0x1ffffc;

	static unsigned int x = 0;
	static unsigned int y = 0;
	unsigned int width = 32;
	while (num_words > 0)
	{
		num_words--;

		unsigned int word = ram->load<unsigned int>(addr);
		
		// TODO do something with this

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
			command_queue.push_back(command);

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

int Gpu::get_cross_product(int x0, int y0, int x1, int y1)
{
	return (x0 * y1) - (y0 * x1);
}

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void Gpu::draw_triangle(int x0, int y0,
	int x1, int y1,
	int x2, int y2,
	unsigned char r, unsigned char g, unsigned char b)
{


	int min_x = std::min(x0, std::min(x1, x2));
	int max_x = std::max(x0, std::max(x1, x2));
	int min_y = std::min(y0, std::min(y1, y2));
	int max_y = std::max(y0, std::max(y1, y2));

	int ex0 = x1 - x0;
	int ey0 = y1 - y0;
	int ex1 = x2 - x0;
	int ey1 = y2 - y0;

	for (int x = min_x; x <= max_x; x++)
	{
		for (int y = min_y; y <= max_y; y++)
		{
			int qx0 = x - x0;
			int qy0 = y - y0;

			int s = get_cross_product(qx0, qy0, ex1, ey1) / get_cross_product(ex0, ey0, ex1, ey1);
			int t = get_cross_product(ex0, ey0, qx0, qy0) / get_cross_product(ex0, ey0, ex1, ey1);

			if (s >= 0 && t >= 0 && (s + t <= 1))
			{
				draw_pixel(x, y, r, g, b);
			}
		}
	}
}

void Gpu::draw_pixel(int x, int y, unsigned char r, unsigned char g, unsigned b)
{
	unsigned int index = ((y*FRAME_WIDTH) + x)*BYTES_PER_PIXEL;

	video_ram[index] = r;
	video_ram[index + 1] = g;
	video_ram[index + 2] = b;
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

unsigned int Gpu::nop()
{
	return 1;
}

unsigned int Gpu::mono_4_pt_opaque()
{
	if (command_queue.size() < 5)
	{
		return 0;
	}

	color_command color;
	vert_command vert1, vert2, vert3, vert4;

	color.value = command_queue[0];
	vert1.value = command_queue[1];
	vert2.value = command_queue[2];
	vert3.value = command_queue[3];
	vert4.value = command_queue[4];

	// triangle 1
	draw_triangle(vert1.x, vert1.y, vert2.x, vert2.y, vert3.x, vert3.y, color.r, color.g, color.b);

	// triangle 2
	draw_triangle(vert2.x, vert2.y, vert3.x, vert3.y, vert4.x, vert4.y, color.r, color.g, color.b);

	return 5;
}

unsigned int Gpu::set_draw_top_left()
{
	// todo
	return 1;
}

unsigned int Gpu::set_draw_bottom_right()
{
	// todo
	return 1;
}

unsigned int Gpu::set_drawing_offset()
{
	// todo
	return 1;
}

unsigned int Gpu::set_draw_mode()
{
	// todo
	return 1;
}

unsigned int Gpu::set_texture_window()
{
	// todo
	return 1;
}

unsigned int Gpu::set_mask_bit()
{
	// todo
	return 1;
}