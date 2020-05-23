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

bool Gpu::is_address_for_device(unsigned int address)
{
	if (address >= GPU_START && address <= GPU_END)
	{
		return true;
	}
	return false;
}

unsigned int Gpu::get_word(unsigned int address)
{
	if (address == GP0_Send_GPUREAD)
	{
		if (gpu_status.ready_vram_to_cpu)
		{
			unsigned int result = copy_next_pixel_from_framebuffer();
			result <<= 16;
			result |= copy_next_pixel_from_framebuffer();

			num_words_to_copy_to_cpu--;
			if (num_words_to_copy_to_cpu == 0)
			{
				gpu_status.ready_vram_to_cpu = false;
			}

			return result;
		}

		return 0;
	}
	else if (address == GP1_Send_GPUSTAT)
	{
		return gpu_status.int_value;
	}

	throw std::out_of_range("address out of range");
}

void Gpu::set_word(unsigned int address, unsigned int value)
{
	if (address == GP0_Send_GPUREAD)
	{
		add_gp0_command(value, false);
	}
	else if (address == GP1_Send_GPUSTAT)
	{
		execute_gp1_command(value);
	}
	else
	{
		throw std::out_of_range("address out of range");
	}
}

Gpu::~Gpu()
{
	if (video_ram)
	{
		delete video_ram;
	}

	if (gp0_fifo)
	{
		delete gp0_fifo;
	}
}

void Gpu::init()
{
	// I have to allocate all the vram memory at runtime or
    // else I get a compiler out of heap space issue at compile time
	video_ram = new unsigned short[VRAM_SIZE];
	memset(video_ram, 0, VRAM_SIZE * sizeof(unsigned short));

	gp0_fifo = new Fifo<unsigned int>(16);

	// hardcoded according to simias guide to get the emulator moving a bit further through the code
	gpu_status.ready_dma = true;
	gpu_status.ready_cmd_word = true;
}

void Gpu::reset()
{
}

void Gpu::tick()
{
}

void Gpu::save_state(std::ofstream& file)
{
	file.write(reinterpret_cast<char*>(&gpu_status.int_value), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(video_ram), sizeof(unsigned short)*VRAM_SIZE);

	std::vector<unsigned int> commands;
	while (gp0_fifo->get_current_size() > 0)
	{
		commands.push_back(gp0_fifo->pop());
	}

	unsigned int num_commands = commands.size();

	file.write(reinterpret_cast<char*>(&num_commands), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(commands.data()), sizeof(unsigned int)*num_commands);
}

void Gpu::load_state(std::ifstream& file)
{
	file.read(reinterpret_cast<char*>(&gpu_status.int_value), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(video_ram), sizeof(unsigned short)*VRAM_SIZE);

	unsigned int num_commands = 0;
	file.read(reinterpret_cast<char*>(&num_commands), sizeof(unsigned int));

	std::vector<unsigned int> commands;
	commands.resize(num_commands);
	file.read(reinterpret_cast<char*>(commands.data()), sizeof(unsigned int)*num_commands);

	gp0_fifo->clear();

	for (auto iter : commands)
	{
		gp0_fifo->push(iter);
	}
}

void Gpu::sync_mode_request(std::shared_ptr<Bus> bus, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	if (channel_control.transfer_direction == 0)
	{
		throw std::logic_error("have not implemented gpu to ram transfer");
	}

	unsigned int num_words = block_control.BS * block_control.BC;
	DMA_address_step step = static_cast<DMA_address_step>(channel_control.memory_address_step);
	unsigned int addr = base_address.memory_address & 0x1ffffc;

	while (num_words > 0)
	{
		num_words--;

		unsigned int word = bus->get_word(addr);

		addr += (step == DMA_address_step::increment ? 4 : -4);
	}
}

void Gpu::sync_mode_linked_list(std::shared_ptr<Bus> bus, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control)
{
	if (channel_control.transfer_direction == 0)
	{
		throw std::logic_error("have not implemented gpu to ram transfer");
	}

	unsigned int addr = base_address.memory_address & 0x1ffffc;
	while (true)
	{
		unsigned int header = bus->get_word(addr);
		unsigned int num_words = header >> 24;

		while (num_words > 0)
		{
			addr = (addr + 4) & 0x1ffffc;

			unsigned int command = bus->get_word(addr);
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
}

void Gpu::execute_gp0_commands()
{
	gp_command top_command = gp0_fifo->peek();
	gp0_commands current_command = static_cast<gp0_commands>(top_command.color.op);
	bool command_executed = false;
	while (gp0_fifo->is_empty() == false)
	{
		switch (current_command)
		{
		case gp0_commands::NOP:
		{
			gp0_fifo->pop();
			command_executed = true;
		} break;

		case gp0_commands::DRAW_MODE:
		{
			command_executed = set_draw_mode();
		} break;

		case gp0_commands::SET_DRAW_TOP_LEFT:
		{
			command_executed = set_draw_top_left();
		} break;

		case gp0_commands::SET_DRAW_BOTTOM_RIGHT:
		{
			command_executed = set_draw_bottom_right();
		} break;

		case gp0_commands::SET_DRAWING_OFFSET:
		{
			command_executed = set_drawing_offset();
		} break;

		case gp0_commands::TEX_WINDOW:
		{
			command_executed = set_texture_window();
		} break;

		case gp0_commands::MASK_BIT:
		{
			command_executed = set_mask_bit();
		} break;

		case gp0_commands::MONO_4_PT_OPAQUE:
		{
			command_executed = mono_4_pt_opaque();
		} break;

		case gp0_commands::CLEAR_CACHE:
		{
			command_executed = clear_cache();
		} break;

		case gp0_commands::COPY_RECT_CPU_VRAM:
		{
			command_executed = copy_rectangle_from_cpu_to_vram();
		} break;

		case gp0_commands::COPY_RECT_VRAM_CPU:
		{
			command_executed = copy_rectangle_from_vram_to_cpu();
		} break;

		case gp0_commands::SHADED_3_PT_OPAQUE:
		{
			command_executed = shaded_3_pt_opaque();
		} break;

		case gp0_commands::SHADED_4_PT_OPAQUE:
		{
			command_executed = shaded_4_pt_opaque();
		} break;

		case gp0_commands::TEX_4_OPAQUE_TEX_BLEND:
		{
			command_executed = tex_4_pt_opaque_blend();
		} break;

		case gp0_commands::FILL_RECT:
		{
			command_executed = fill_rect();
		} break;

		default:
			throw std::logic_error("not implemented");
		}

		if (false == command_executed)
		{
			break;
		}
	}
}

void Gpu::add_gp0_command(gp_command command, bool via_dma)
{
	if (num_words_to_copy_to_gpu == 0)
	{
		gp0_fifo->push(command.raw);
		execute_gp0_commands();
	}
	else
	{
		copy_next_pixel_to_framebuffer(command.pixel_data.data0);
		copy_next_pixel_to_framebuffer(command.pixel_data.data1);
		num_words_to_copy_to_gpu--;
	}
}

void Gpu::execute_gp1_command(gp_command command)
{	
	switch (static_cast<gp1_commands>(command.color.op))
	{
		case gp1_commands::RESET_GPU:
		{
			// todo
		} break;

		case gp1_commands::RESET_COMMAND_BUFFER:
		{
			gp0_fifo->clear();
		} break;

		case gp1_commands::ACK_IRQ1:
		{
			// todo
		} break;

		case gp1_commands::DISPLAY_ENABLE:
		{
			// 0 = on, 1 = off bizarrely
			gpu_status.display_enable = 0x00000001 & command.raw;
		} break;

		case gp1_commands::DMA_DIR:
		{
			gpu_status.dma_direction = 0x00000003 & command.raw;
		} break;

		case gp1_commands::START_OF_DISPLAY:
		{
			// todo
		} break;

		case gp1_commands::HOR_DISPLAY_RANGE:
		{
			// todo
		} break;

		case gp1_commands::VERT_DISPLAY_RANGE:
		{
			// todo
		} break;

		case gp1_commands::DISPLAY_MODE:
		{
			// Some of these values if set cause the psx to get stuck in an infinite loop at startup
			//gpu_status.h_res_1 = command.display_mode.horizontal_res;
			//gpu_status.v_res = command.display_mode.vertical_res;
			gpu_status.video_mode = command.display_mode.video_mode;
			//gpu_status.display_depth = command.display_mode.display_color_depth;
			//gpu_status.v_interlace = command.display_mode.vertical_interlace;
			//gpu_status.h_res_2 = command.display_mode.horizontal_res_2;
			//gpu_status.reverse = command.display_mode.reverse_flag;
		} break;

		case gp1_commands::GET_GPU_INFO:
		{
			// todo
		} break;

		default:
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

void Gpu::draw_rectangle(glm::ivec2 top_left, glm::ivec2 width_height, glm::u8vec3 rgb)
{
	for (int y = top_left.y; y <= (top_left.y + width_height.y); y++)
	{
		for (int x = top_left.x; x <= (top_left.x + width_height.x); x++)
		{
			glm::ivec2 cur_pos = glm::ivec2(x, y);
			// fill rect ignores the pixel draw area
			draw_pixel(cur_pos, rgb, true);
		}
	}
}

void Gpu::draw_pixel(glm::ivec2 v, glm::u8vec3 rgb, bool ignore_draw_offset)
{
	int x = static_cast<int>(v.x) + x_offset;
	int y = static_cast<int>(v.y) + y_offset;

	if (ignore_draw_offset == false)
	{
		if (x < static_cast<int>(draw_area_min_x) || x > static_cast<int>(draw_area_max_x))
		{
			return;
		}

		if (y < static_cast<int>(draw_area_min_y) || y > static_cast<int>(draw_area_max_y))
		{
			return;
		}
	}

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

void Gpu::copy_next_pixel_to_framebuffer(unsigned int short pixel_data)
{
	unsigned int x = copy_to_gpu_current_coord.dest_coord.x_pos;
	unsigned int y = copy_to_gpu_current_coord.dest_coord.y_pos;

	if ((x >= 0 && x < FRAME_WIDTH) && (y >= 0 && y < FRAME_HEIGHT))
	{
		unsigned int index = ((y*FRAME_WIDTH) + x);
		video_ram[index] = pixel_data;
	}

	x++;
	if ((x - copy_to_gpu_dest_coord.dest_coord.x_pos) >= copy_to_gpu_width_height.dims.x_siz)
	{
		x = copy_to_gpu_dest_coord.dest_coord.x_pos;
		y++;
	}

	copy_to_gpu_current_coord.dest_coord.x_pos = x;
	copy_to_gpu_current_coord.dest_coord.y_pos = y;
}

unsigned short Gpu::copy_next_pixel_from_framebuffer()
{
	unsigned int x = copy_to_cpu_current_coord.dest_coord.x_pos;
	unsigned int y = copy_to_cpu_current_coord.dest_coord.y_pos;

	unsigned short result = 0;
	if ((x >= 0 && x < FRAME_WIDTH) && (y >= 0 && y < FRAME_HEIGHT))
	{
		unsigned int index = ((y*FRAME_WIDTH) + x);
		result = video_ram[index];
	}

	x++;
	if ((x - copy_to_cpu_src_coord.dest_coord.x_pos) >= copy_to_cpu_width_height.dims.x_siz)
	{
		x = copy_to_cpu_src_coord.dest_coord.x_pos;
		y++;
	}

	copy_to_cpu_current_coord.dest_coord.x_pos = x;
	copy_to_cpu_current_coord.dest_coord.y_pos = y;

	return result;
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

bool Gpu::mono_4_pt_opaque()
{
	if (gp0_fifo->get_current_size() < 5)
	{
		return false;
	}

	gp_command color_command = gp0_fifo->pop();
	glm::u8vec3 rgb(color_command.color.r, color_command.color.g, color_command.color.b);

	gp_command vert0_command = gp0_fifo->pop();
	glm::ivec2 v0(vert0_command.vert.x, vert0_command.vert.y);

	gp_command vert1_command = gp0_fifo->pop();
	glm::ivec2 v1(vert1_command.vert.x, vert1_command.vert.y);

	gp_command vert2_command = gp0_fifo->pop();
	glm::ivec2 v2(vert2_command.vert.x, vert2_command.vert.y);

	gp_command vert3_command = gp0_fifo->pop();
	glm::ivec2 v3(vert3_command.vert.x, vert3_command.vert.y);

	// triangle 1
	draw_triangle(v0, v1, v2, rgb, rgb, rgb);

	// triangle 2
	draw_triangle(v1, v2, v3, rgb, rgb, rgb);

	return true;
}

bool Gpu::shaded_3_pt_opaque()
{
	if (gp0_fifo->get_current_size() < 6)
	{
		return false;
	}

	gp_command color0_command = gp0_fifo->pop();
	gp_command vert0_command = gp0_fifo->pop();
	glm::u8vec3 rgb0(color0_command.color.r, color0_command.color.g, color0_command.color.b);
	glm::ivec2 v0(vert0_command.vert.x, vert0_command.vert.y);

	gp_command color1_command = gp0_fifo->pop();
	gp_command vert1_command = gp0_fifo->pop();
	glm::u8vec3 rgb1(color1_command.color.r, color1_command.color.g, color1_command.color.b);
	glm::ivec2 v1(vert1_command.vert.x, vert1_command.vert.y);

	gp_command color2_command = gp0_fifo->pop();
	gp_command vert2_command = gp0_fifo->pop();
	glm::u8vec3 rgb2(color2_command.color.r, color2_command.color.g, color2_command.color.b);
	glm::ivec2 v2(vert2_command.vert.x, vert2_command.vert.y);

	// triangle 1
	draw_triangle(v0, v1, v2, rgb0, rgb1, rgb2);

	return true;
}

bool Gpu::shaded_4_pt_opaque()
{
	if (gp0_fifo->get_current_size() < 8)
	{
		return false;
	}

	gp_command color0_command = gp0_fifo->pop();
	gp_command vert0_command = gp0_fifo->pop();
	glm::u8vec3 rgb0(color0_command.color.r, color0_command.color.g, color0_command.color.b);
	glm::ivec2 v0(vert0_command.vert.x, vert0_command.vert.y);

	gp_command color1_command = gp0_fifo->pop();
	gp_command vert1_command = gp0_fifo->pop();
	glm::u8vec3 rgb1(color1_command.color.r, color1_command.color.g, color1_command.color.b);
	glm::ivec2 v1(vert1_command.vert.x, vert1_command.vert.y);

	gp_command color2_command = gp0_fifo->pop();
	gp_command vert2_command = gp0_fifo->pop();
	glm::u8vec3 rgb2(color2_command.color.r, color2_command.color.g, color2_command.color.b);
	glm::ivec2 v2(vert2_command.vert.x, vert2_command.vert.y);

	gp_command color3_command = gp0_fifo->pop();
	gp_command vert3_command = gp0_fifo->pop();
	glm::u8vec3 rgb3(color3_command.color.r, color3_command.color.g, color3_command.color.b);
	glm::ivec2 v3(vert3_command.vert.x, vert3_command.vert.y);

	// triangle 1
	draw_triangle(v0, v1, v2, rgb0, rgb1, rgb2);

	// triangle 2
	draw_triangle(v1, v2, v3, rgb1, rgb2, rgb3);

	return true;
}

bool Gpu::tex_4_pt_opaque_blend()
{
	if (gp0_fifo->get_current_size() < 9)
	{
		return false;
	}

	gp_command color_command = gp0_fifo->pop();
	glm::u8vec3 rgb(color_command.color.r, color_command.color.g, color_command.color.b);

	gp_command vert0_command = gp0_fifo->pop();
	glm::ivec2 v0(vert0_command.vert.x, vert0_command.vert.y);
	gp_command texcoord0_palette = gp0_fifo->pop();

	gp_command vert1_command = gp0_fifo->pop();
	glm::ivec2 v1(vert1_command.vert.x, vert1_command.vert.y);
	gp_command texcoord1_page = gp0_fifo->pop();

	gp_command vert2_command = gp0_fifo->pop();
	glm::ivec2 v2(vert2_command.vert.x, vert2_command.vert.y);
	gp_command texcoord2 = gp0_fifo->pop();

	gp_command vert3_command = gp0_fifo->pop();
	glm::ivec2 v3(vert3_command.vert.x, vert3_command.vert.y);
	gp_command texcoord3 = gp0_fifo->pop();

	// triangle 1
	draw_triangle(v0, v1, v2, rgb, rgb, rgb);

	// triangle 2
	draw_triangle(v1, v2, v3, rgb, rgb, rgb);

	return true;
}

bool Gpu::fill_rect()
{
	if (gp0_fifo->get_current_size() < 3)
	{
		return false;
	}

	gp_command color_command = gp0_fifo->pop();
	glm::u8vec3 rgb(color_command.color.r, color_command.color.g, color_command.color.b);

	gp_command vert_command = gp0_fifo->pop();
	glm::ivec2 top_left(vert_command.vert.x, vert_command.vert.y);

	gp_command dim_command = gp0_fifo->pop();
	glm::ivec2 width_height(dim_command.dims.x_siz, dim_command.dims.y_siz);

	draw_rectangle(top_left, width_height, rgb);

	return true;
}

bool Gpu::set_draw_top_left()
{
	gp_command top_left(gp0_fifo->pop());

	draw_area_min_x = top_left.draw_area.x_coord;
	draw_area_min_y = top_left.draw_area.y_coord;

	return true;
}

bool Gpu::set_draw_bottom_right()
{
	gp_command bottom_right(gp0_fifo->pop());

	draw_area_max_x = bottom_right.draw_area.x_coord;
	draw_area_max_y = bottom_right.draw_area.y_coord;

	return true;
}

bool Gpu::set_drawing_offset()
{
	gp_command offset(gp0_fifo->pop());

	x_offset = offset.draw_offset.x_offset;
	y_offset = offset.draw_offset.y_offset;

	return true;
}

bool Gpu::set_draw_mode()
{
	// the command only sets about half the gpu status values
	// it conveniently follows the same bit pattern up until texture disable
	// which I believe we can ignore according to the problemkaputt.de documentation
	gpu_status_union new_status(gp0_fifo->pop());

	gpu_status.tex_page_x_base = new_status.tex_page_x_base;
	gpu_status.tex_page_y_base = new_status.tex_page_y_base;
	gpu_status.semi_transparency = new_status.semi_transparency;
	gpu_status.tex_page_colors = new_status.tex_page_colors;
	gpu_status.dither = new_status.dither;
	gpu_status.drawing_to_display_area = new_status.drawing_to_display_area;

	// ignoring all other values for the moment

	return true;
}

bool Gpu::set_texture_window()
{
	gp0_fifo->pop();
	// todo
	return true;
}

bool Gpu::set_mask_bit()
{
	gp0_fifo->pop();
	// todo
	return true;
}

bool Gpu::clear_cache()
{
	gp0_fifo->pop();
	// todo
	return true;
}

bool Gpu::copy_rectangle_from_cpu_to_vram()
{
	if (gp0_fifo->get_current_size() >= 3)
	{
		gp0_fifo->pop();
		copy_to_gpu_current_coord = copy_to_gpu_dest_coord = gp0_fifo->pop();
		copy_to_gpu_width_height = gp0_fifo->pop();

		unsigned int num_halfwords_to_copy = copy_to_gpu_width_height.dims.x_siz*copy_to_gpu_width_height.dims.y_siz;
		num_words_to_copy_to_gpu = num_halfwords_to_copy / 2;

		// if an odd number of halfwords, an extra padding halfword will be added
		if (num_halfwords_to_copy % 2)
		{
			num_words_to_copy_to_gpu += 1;
		}

		return true;
	}

	return false;
}

bool Gpu::copy_rectangle_from_vram_to_cpu()
{
	if (gp0_fifo->get_current_size() >= 3)
	{
		gp0_fifo->pop();
		copy_to_cpu_src_coord = copy_to_cpu_current_coord = gp0_fifo->pop();
		copy_to_cpu_width_height = gp0_fifo->pop();

		unsigned int num_halfwords_to_copy = copy_to_cpu_width_height.dims.x_siz*copy_to_cpu_width_height.dims.y_siz;
		num_words_to_copy_to_cpu = num_halfwords_to_copy / 2;

		// if an odd number of halfwords, an extra padding halfword will be added
		if (num_halfwords_to_copy % 2)
		{
			num_words_to_copy_to_cpu += 1;
		}

		gpu_status.ready_vram_to_cpu = true;
		return true;
	}

	return false;
}