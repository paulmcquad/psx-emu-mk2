#include "Gpu.hpp"
#include "Ram.hpp"
#include "InstructionEnums.hpp"
#include "InstructionTypes.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/common.hpp>

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
				add_gp0_command(command, false);
			} break;
			case GP1_SEND:
			{
				execute_gp1_command(command);
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

	// GP0 commands
	gp0_command_map[gp0_commands::NOP] = &Gpu::nop;
	gp0_command_map[gp0_commands::SET_DRAW_TOP_LEFT] = &Gpu::set_draw_top_left;
	gp0_command_map[gp0_commands::SET_DRAW_BOTTOM_RIGHT] = &Gpu::set_draw_bottom_right;
	gp0_command_map[gp0_commands::SET_DRAWING_OFFSET] = &Gpu::set_drawing_offset;

	gp0_command_map[gp0_commands::DRAW_MODE] = &Gpu::set_draw_mode;
	gp0_command_map[gp0_commands::TEX_WINDOW] = &Gpu::set_texture_window;
	gp0_command_map[gp0_commands::MASK_BIT] = &Gpu::set_mask_bit;
	gp0_command_map[gp0_commands::CLEAR_CACHE] = &Gpu::clear_cache;

	gp0_command_map[gp0_commands::COPY_RECT_CPU_VRAM] = &Gpu::copy_rectangle_from_cpu_to_vram;
	gp0_command_map[gp0_commands::COPY_RECT_VRAM_CPU] = &Gpu::copy_rectangle_from_vram_to_cpu;

	gp0_command_map[gp0_commands::SHADED_3_PT_OPAQUE] = &Gpu::shaded_3_pt_opaque;

	gp0_command_map[gp0_commands::MONO_4_PT_OPAQUE] = &Gpu::mono_4_pt_opaque;
	gp0_command_map[gp0_commands::SHADED_4_PT_OPAQUE] = &Gpu::shaded_4_pt_opaque;
	gp0_command_map[gp0_commands::TEX_4_OPAQUE_TEX_BLEND] = &Gpu::tex_4_pt_opaque_blend;

	// GP1 commands
	gp1_command_map[gp1_commands::RESET_GPU] = &Gpu::reset_gpu;
	gp1_command_map[gp1_commands::RESET_COMMAND_BUFFER] = &Gpu::reset_command_buffer;
	gp1_command_map[gp1_commands::ACK_IRQ1] = &Gpu::ack_gpu_interrupt;
	gp1_command_map[gp1_commands::DISPLAY_ENABLE] = &Gpu::display_enable;
	gp1_command_map[gp1_commands::DMA_DIR] = &Gpu::dma_direction;
	gp1_command_map[gp1_commands::START_OF_DISPLAY] = &Gpu::start_display_area;
	gp1_command_map[gp1_commands::HOR_DISPLAY_RANGE] = &Gpu::horizontal_display_range;
	gp1_command_map[gp1_commands::VERT_DISPLAY_RANGE] = &Gpu::vertical_display_range;
	gp1_command_map[gp1_commands::DISPLAY_MODE] = &Gpu::display_mode;
	gp1_command_map[gp1_commands::GET_GPU_INFO] = &Gpu::get_gpu_info;
}

void Gpu::reset()
{
}

void Gpu::tick()
{
	execute_gp0_commands();
}

void Gpu::save_state(std::ofstream& file)
{
	file.write(reinterpret_cast<char*>(&gpu_status.int_value), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(video_ram.data()), sizeof(unsigned short)*video_ram.size());

	std::vector<unsigned int> commands;
	for (auto iter : gp0_command_queue)
	{
		commands.push_back(iter);
	}

	unsigned int num_commands = commands.size();

	file.write(reinterpret_cast<char*>(&num_commands), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(commands.data()), sizeof(unsigned int)*num_commands);
}

void Gpu::load_state(std::ifstream& file)
{
	file.read(reinterpret_cast<char*>(&gpu_status.int_value), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(video_ram.data()), sizeof(unsigned short)*video_ram.size());

	unsigned int num_commands = 0;
	file.read(reinterpret_cast<char*>(&num_commands), sizeof(unsigned int));

	std::vector<unsigned int> commands;
	commands.resize(num_commands);
	file.read(reinterpret_cast<char*>(commands.data()), sizeof(unsigned int)*num_commands);

	gp0_command_queue.clear();

	for (auto iter : commands)
	{
		gp0_command_queue.push_back(iter);
	}
}

void Gpu::sync_mode_request(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	//std::cout << "Starting GPU request DMA\n";
	unsigned int num_words = block_control.BS * block_control.BC;
	DMA_address_step step = static_cast<DMA_address_step>(channel_control.memory_address_step);
	unsigned int addr = base_address.memory_address & 0x1ffffc;

	while (num_words > 0)
	{
		num_words--;

		unsigned int word = ram->load_word(addr);

		addr += (step == DMA_address_step::increment ? 4 : -4);
	}
	//std::cout << "Finished GPU request DMA\n";
}

void Gpu::sync_mode_linked_list(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	//std::cout << "Starting GPU linked list DMA\n";
	unsigned int addr = base_address.memory_address & 0x1ffffc;
	while (true)
	{
		unsigned int header = ram->load_word(addr);
		unsigned int num_words = header >> 24;

		while (num_words > 0)
		{
			addr = (addr + 4) & 0x1ffffc;

			unsigned int command = ram->load_word(addr);
			add_gp0_command(command, true);

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
	//std::cout << "Finished GPU linked list DMA\n";
}

void Gpu::execute_gp0_commands()
{
	while (gp0_command_queue.empty() == false)
	{
		color_command command(gp0_command_queue.front());
		gp0_commands current_command = static_cast<gp0_commands>(command.op);

		// let's assume it always finds the function ptr :)
		auto iter = gp0_command_map.find(current_command);
		if (iter != gp0_command_map.end())
		{
			// execute the command
			unsigned int commands_to_remove = (this->*iter->second)();
			// cleanup
			if (commands_to_remove == 0)
			{
				// waiting for more words
				break;
			}
			while (commands_to_remove > 0)
			{
				if (gp0_command_queue.empty())
				{
					throw std::out_of_range("removed too many commands from queue");
				}
				gp0_command_queue.pop_front();
				commands_to_remove--;
			}
		}
		else
		{
			std::cout << "GP0: " << std::hex << command.value << std::endl;
			throw std::logic_error("not implemented");
		}
	}
}

void Gpu::add_gp0_command(unsigned int command, bool via_dma)
{
	gp0_command_queue.push_back(command);
}

void Gpu::execute_gp1_command(unsigned int command)
{
	color_command gp1_command(command);
	gp1_commands current_command = static_cast<gp1_commands>(gp1_command.op);
	auto iter = gp1_command_map.find(current_command);
	if (iter != gp1_command_map.end())
	{
		(this->*iter->second)(command);
	}
	else
	{
		std::cout << "GP1: " << std::hex << gp1_command.value << std::endl;
		throw std::logic_error("not implemented");
	}
}

// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
// https://codeplea.com/triangular-interpolation
void Gpu::draw_triangle(glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2, glm::u8vec3 rgb0, glm::u8vec3 rgb1, glm::u8vec3 rgb2)
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
			glm::vec3 w = calc_barycentric(cur_pos, v0, v1, v2);

			if (w[0] >= 0 && w[1] >= 0 && (w[0] + w[1] <= 1))
			{
				glm::u8vec3 rgb;
				rgb.r = rgb0.r*w[0] + rgb1.r*w[1] + rgb2.r*w[2];
				rgb.g = rgb0.g*w[0] + rgb1.g*w[1] + rgb2.g*w[2];
				rgb.b = rgb0.b*w[0] + rgb1.b*w[1] + rgb2.b*w[2];

				draw_pixel(cur_pos, rgb);
			}
		}
	}
}

void Gpu::draw_pixel(glm::ivec2 v, glm::u8vec3 rgb)
{
	unsigned int x = v.x;
	unsigned int y = v.y;

	if (x >= 0 && x < width)
	{
		if (y >= 0 && y < height)
		{
			unsigned int index = ((y*FRAME_WIDTH) + x);
			unsigned short colour_16 = (rgb.r >> 3) | ((rgb.g >> 2) << 5) | ((rgb.b >> 3) << 11);
			video_ram[index] = colour_16;
		}
	}
}

// https://codeplea.com/triangular-interpolation
glm::vec3 Gpu::calc_barycentric(glm::ivec2 pos, glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2)
{
	glm::vec3 w;

	float d = (v1.y - v2.y)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.y - v2.y);

	w[0] = (v1.y - v2.y)*(pos.x - v2.x) + (v2.x - v1.x)*(pos.y - v2.y);
	w[0] /= d;

	w[1] = (v2.y - v0.y)*(pos.x - v2.x) + (v0.x - v2.x)*(pos.y - v2.y);
	w[1] /= d;

	w[2] = 1 - w[0] - w[1];

	return w;
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

	color_command color(gp0_command_queue[0]);
	vert_command vert0(gp0_command_queue[1]);
	vert_command vert1(gp0_command_queue[2]);
	vert_command vert2(gp0_command_queue[3]);
	vert_command vert3(gp0_command_queue[4]);

	glm::ivec2 v0(vert0.x, vert0.y);
	glm::ivec2 v1(vert1.x, vert1.y);
	glm::ivec2 v2(vert2.x, vert2.y);
	glm::ivec2 v3(vert3.x, vert3.y);

	glm::u8vec3 rgb(color.r, color.g, color.b);

	// triangle 1
	draw_triangle(v0, v1, v2, rgb, rgb, rgb);

	// triangle 2
	draw_triangle(v1, v2, v3, rgb, rgb, rgb);

	return 5;
}

unsigned int Gpu::shaded_3_pt_opaque()
{
	if (gp0_command_queue.size() < 6)
	{
		return 0;
	}

	color_command color0(gp0_command_queue[0]);
	color_command color1(gp0_command_queue[2]);
	color_command color2(gp0_command_queue[4]);

	vert_command vert0(gp0_command_queue[1]);
	vert_command vert1(gp0_command_queue[3]);
	vert_command vert2(gp0_command_queue[5]);

	glm::ivec2 v0(vert0.x, vert0.y);
	glm::ivec2 v1(vert1.x, vert1.y);
	glm::ivec2 v2(vert2.x, vert2.y);

	glm::u8vec3 rgb0(color0.r, color0.g, color0.b);
	glm::u8vec3 rgb1(color1.r, color1.g, color1.b);
	glm::u8vec3 rgb2(color2.r, color2.g, color2.b);

	// triangle 1
	draw_triangle(v0, v1, v2, rgb0, rgb1, rgb2);

	return 6;
}

unsigned int Gpu::shaded_4_pt_opaque()
{
	if (gp0_command_queue.size() < 8)
	{
		return 0;
	}

	color_command color0(gp0_command_queue[0]);
	color_command color1(gp0_command_queue[2]);
	color_command color2(gp0_command_queue[4]);
	color_command color3(gp0_command_queue[6]);

	vert_command vert0(gp0_command_queue[1]);
	vert_command vert1(gp0_command_queue[3]);
	vert_command vert2(gp0_command_queue[5]);
	vert_command vert3(gp0_command_queue[7]);

	glm::ivec2 v0(vert0.x, vert0.y);
	glm::ivec2 v1(vert1.x, vert1.y);
	glm::ivec2 v2(vert2.x, vert2.y);
	glm::ivec2 v3(vert3.x, vert3.y);

	glm::u8vec3 rgb0(color0.r, color0.g, color0.b);
	glm::u8vec3 rgb1(color1.r, color1.g, color1.b);
	glm::u8vec3 rgb2(color2.r, color2.g, color2.b);
	glm::u8vec3 rgb3(color3.r, color3.g, color3.b);

	// triangle 1
	draw_triangle(v0, v1, v2, rgb0, rgb1, rgb2);

	// triangle 2
	draw_triangle(v1, v2, v3, rgb1, rgb2, rgb3);

	return 8;
}

unsigned int Gpu::tex_4_pt_opaque_blend()
{
	if (gp0_command_queue.size() < 9)
	{
		return 0;
	}

	// todo implement

	return 9;
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
	draw_offset_command offset(gp0_command_queue.front());

	x_offset = offset.x_offset;
	y_offset = offset.y_offset;

	return 1;
}

unsigned int Gpu::set_draw_mode()
{
	// the command only sets about half the gpu status values
	// it conveniently follows the same bit pattern up until texture disable
	// which I believe we can ignore according to the problemkaputt.de documentation
	gpu_status_union new_status(gp0_command_queue.front());

	gpu_status.tex_page_x_base = new_status.tex_page_x_base;
	gpu_status.tex_page_y_base = new_status.tex_page_y_base;
	gpu_status.semi_transparency = new_status.semi_transparency;
	gpu_status.tex_page_colors = new_status.tex_page_colors;
	gpu_status.dither = new_status.dither;
	gpu_status.drawing_to_display_area = new_status.drawing_to_display_area;

	// ignoring all other values for the moment

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
	if (gp0_command_queue.size() >= 3)
	{
		dest_coord_command dest_coord(gp0_command_queue[1]);
		width_height_command width_height(gp0_command_queue[2]);
		unsigned int num_halfwords_to_copy = width_height.x_siz*width_height.y_siz;
		// round up as there should be padding if the number of halfwords is odd
		unsigned int num_words_to_copy = ceil(num_halfwords_to_copy / 2.0);

		unsigned int expected_size = num_words_to_copy + 3;
		if (gp0_command_queue.size() >=  expected_size)
		{
			// we are just going to pop the data of the deque in this function to
			// simply things
			gp0_command_queue.pop_front();
			gp0_command_queue.pop_front();
			gp0_command_queue.pop_front();
			
			unsigned int x = 0;
			unsigned int y = 0;
			while (num_words_to_copy)
			{
				unsigned int data = gp0_command_queue.front();
				gp0_command_queue.pop_front();

				// todo verify
				/*for (int halfword_offset = 0; halfword_offset < 2; halfword_offset++)
				{
					unsigned short colour_16 = data >> (16 * halfword_offset);
					unsigned int index = ((y*FRAME_WIDTH) + x);
					video_ram[index] = colour_16;

					x++;
					if (x >= width_height.x_siz)
					{
						x = 0;
						y++;
					}
				}*/

				num_words_to_copy--;
			}
		}
	}

	return 0;
}

unsigned int Gpu::copy_rectangle_from_vram_to_cpu()
{
	if (gp0_command_queue.size() >= 3)
	{
		gpu_status.ready_vram_to_cpu = true;
		// todo implement
		return 3;
	}

	return 0;
}

void Gpu::reset_gpu(unsigned int command)
{

}

void Gpu::reset_command_buffer(unsigned int command)
{
	gp0_command_queue.clear();
}

void Gpu::ack_gpu_interrupt(unsigned int command)
{

}

void Gpu::display_enable(unsigned int command)
{
	// 0 = on, 1 = off bizarrely
	gpu_status.display_enable = 0x00000001 & command;
}

void Gpu::dma_direction(unsigned int command)
{
	gpu_status.dma_direction = 0x00000003 & command;
}

void Gpu::start_display_area(unsigned int command)
{

}

void Gpu::horizontal_display_range(unsigned int command)
{

}

void Gpu::vertical_display_range(unsigned int command)
{

}

void Gpu::display_mode(unsigned int command)
{

}

void Gpu::get_gpu_info(unsigned int command)
{

}

void Gpu::new_texture_disable(unsigned int command)
{

}

void Gpu::special_texture_disable(unsigned int command)
{

}