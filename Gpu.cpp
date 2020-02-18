#include "Gpu.hpp"
#include "MemoryMap.hpp"
#include "InstructionEnums.hpp"
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/common.hpp>

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
		glm::u8vec3 rgb;
		unsigned char op;
	};
};

unsigned char Gpu::get(gpu_registers reg_name, unsigned int byte_offset)
{
	switch (reg_name)
	{
		case GPUSTAT:
		{
			return gpu_status.byte_value[byte_offset];
		} break;
		case GPUREAD:
		{
			return gpu_read.byte_value[byte_offset];
		} break;
		default:
			throw std::logic_error("can't read this gpu");
	}
}

void Gpu::set(gpu_registers reg_name, unsigned int byte_offset, unsigned char value)
{
	// command is set byte by byte but expected to be added to the command
	// queue as a word value, so we store it in a static variable and put it
	// in the list when its finished 
	static unsigned char command_bytes[4] = { 0 };
	command_bytes[byte_offset] = value;

	if (byte_offset == 3)
	{
		unsigned int command = 0;
		memcpy(&command, &command_bytes, sizeof(unsigned int));
		switch (reg_name)
		{
			case GP0_SEND:
			{
				gp0_command_queue.push_back(command);
			} break;
			case GP1_SEND:
			{
				gp1_command_queue.push_back(command);
			} break;
			default:
				throw std::logic_error("can't write to this gpu register");
		}
	}
}

void Gpu::init()
{
	// I have to allocate all the vram memory at runtime or
    // else I get a compiler out of heap space issue at compile time
	video_ram.resize(VRAM_SIZE, 0x0);
	// hardcoded according to simias guide to get the emulator moving a bit further through the code
	gpu_status.ready_dma = true;
	gpu_status.ready_cmd_word = true;

	gp0_command_map[gp0_commands::NOP] = &Gpu::nop;
	gp0_command_map[gp0_commands::SET_DRAW_TOP_LEFT] = &Gpu::set_draw_top_left;
	gp0_command_map[gp0_commands::SET_DRAW_BOTTOM_RIGHT] = &Gpu::set_draw_bottom_right;
	gp0_command_map[gp0_commands::SET_DRAWING_OFFSET] = &Gpu::set_drawing_offset;

	gp0_command_map[gp0_commands::DRAW_MODE] = &Gpu::set_draw_mode;
	gp0_command_map[gp0_commands::TEX_WINDOW] = &Gpu::set_texture_window;
	gp0_command_map[gp0_commands::MASK_BIT] = &Gpu::set_mask_bit;
	gp0_command_map[gp0_commands::CLEAR_CACHE] = &Gpu::clear_cache;

	gp0_command_map[gp0_commands::COPY_RECT_CPU_VRAM] = &Gpu::copy_rectangle_from_cpu_to_vram;

	gp0_command_map[gp0_commands::MONO_4_PT_OPAQUE] = &Gpu::mono_4_pt_opaque;
}

void Gpu::reset()
{
	
}

void Gpu::tick()
{
	while (gp0_command_queue.empty() == false)
	{
		color_command command;
		command.value = gp0_command_queue.front();

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
				gp0_command_queue.pop_front();
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
			gp0_command_queue.push_back(command);

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

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void Gpu::draw_triangle(glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2, glm::u8vec3 rgb)
{
	glm::vec2 e0 = v1 - v0;
	glm::vec2 e1 = v2 - v0;

	// does component wise min/max operations
	glm::ivec2 min_val = glm::min(v0, glm::min(v1, v2));
	glm::ivec2 max_val = glm::max(v0, glm::max(v1, v2));

	for (int y = min_val.y; y <= max_val.y; y++)
	{
		for (int x = min_val.x; x <= max_val.x; x++)
		{
			glm::ivec2 cur_pos = glm::ivec2(x, y);
			glm::vec2 q = cur_pos - v0;

			// glm::determinant(glm::dmat2(a, b)) is equivalent of a,b cross product in 2d
			float s = glm::determinant(glm::dmat2(q, e1)) / glm::determinant(glm::dmat2(e0, e1));
			float t = glm::determinant(glm::dmat2(e0, q)) / glm::determinant(glm::dmat2(e0, e1));

			if (s >= 0 && t >= 0 && (s + t <= 1))
			{
				draw_pixel(cur_pos, rgb);
			}
		}
	}
}

void Gpu::draw_pixel(glm::ivec2 v, glm::u8vec3 rgb)
{
	unsigned int index = ((v.y*FRAME_WIDTH) + v.x)*BYTES_PER_PIXEL;

	video_ram[index] = rgb.r;
	video_ram[index + 1] = rgb.b;
	video_ram[index + 2] = rgb.g;
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
	if (gp0_command_queue.size() < 5)
	{
		return 0;
	}

	color_command color;
	vert_command vert0, vert1, vert2, vert3;

	color.value = gp0_command_queue[0];
	vert0.value = gp0_command_queue[1];
	vert1.value = gp0_command_queue[2];
	vert2.value = gp0_command_queue[3];
	vert3.value = gp0_command_queue[4];

	glm::ivec2 v0(vert0.x, vert0.y);
	glm::ivec2 v1(vert1.x, vert1.y);
	glm::ivec2 v2(vert2.x, vert2.y);
	glm::ivec2 v3(vert3.x, vert3.y);

	// triangle 1
	draw_triangle(v0, v1, v2, color.rgb);

	// triangle 2
	draw_triangle(v1, v2, v3, color.rgb);

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

unsigned int Gpu::clear_cache()
{
	// todo
	return 1;
}

unsigned int Gpu::copy_rectangle_from_cpu_to_vram()
{
	if (gp0_command_queue.size() < 3)
	{
		return 0;
	}


}