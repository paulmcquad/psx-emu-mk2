#pragma once
#include <vector>
#include <deque>
#include <unordered_map>
#include <glm/fwd.hpp>
#include "Dma.hpp"

constexpr unsigned int FRAME_WIDTH = 1024;
constexpr unsigned int FRAME_HEIGHT = 512;
constexpr unsigned int BYTES_PER_PIXEL = 3; // psx is rgb565 but its simpler if I make it rgb888
constexpr unsigned int VRAM_SIZE = FRAME_WIDTH * FRAME_HEIGHT * BYTES_PER_PIXEL;

// don't know why this isn't defined
constexpr unsigned int GL_RGB565 = 0x8D62;

enum class gp0_commands : unsigned char;
enum class gp1_commands : unsigned char;

class Gpu : public DMA_interface
{
public:

	void init();
	void reset();
	void tick();

	virtual void sync_mode_request(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;
	virtual void sync_mode_linked_list(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;

	enum gpu_registers
	{
		GP0_SEND,
		GP1_SEND,
		GPUREAD,
		GPUSTAT
	};

	unsigned char get(gpu_registers reg_name, unsigned int byte_offset);
	void set(gpu_registers reg_name, unsigned int byte_offset, unsigned char value);

	union
	{
		unsigned int int_value;
		unsigned char byte_value[4];
	} gpu_read;

	union
	{
		unsigned int int_Value;
		unsigned char byte_value[4];
		struct
		{
			unsigned int tex_page_x_base : 4;
			unsigned int tex_page_y_base : 1;
			unsigned int semi_transparency : 2;
			unsigned int tex_page_colors : 2;
			unsigned int dither : 1;
			unsigned int drawing_to_display_area : 1;
			unsigned int set_mask_bit : 1;
			unsigned int draw_pixels : 1;
			unsigned int interlace_field : 1;
			unsigned int reverse : 1;
			unsigned int tex_disable : 1;
			unsigned int h_res_2 : 1;
			unsigned int h_res_1 : 2;
			unsigned int v_res : 1;
			unsigned int video_mode : 1;
			unsigned int display_depth : 1;
			unsigned int v_interlace : 1;
			unsigned int display_enable : 1;
			unsigned int irq_request : 1;
			unsigned int dma_request : 1;
			unsigned int ready_cmd_word : 1;
			unsigned int ready_vram_to_cpu : 1;
			unsigned int ready_dma : 1;
			unsigned int dma_direction : 2;
			unsigned int even_odd : 1;
		};
	} gpu_status;
	

	std::vector<unsigned char> video_ram;
	// apparently the psx has a 16 word FIFO queue, i'm going to work under the assumption
	// that I can ignore that
	std::deque<unsigned int> gp0_command_queue;
	std::deque<unsigned int> gp1_command_queue;
	unsigned int width = FRAME_WIDTH;
	unsigned int height = FRAME_HEIGHT;

private:
	std::unordered_map<gp0_commands, unsigned int (Gpu::*)()> gp0_command_map;
	std::unordered_map<gp1_commands, unsigned int (Gpu::*)()> gp1_command_map;

	void draw_triangle(glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2, glm::u8vec3 rgb);
	void draw_pixel(glm::ivec2 v, glm::u8vec3 rgb);

	void draw_static();

	unsigned int nop();
	unsigned int set_draw_top_left();
	unsigned int set_draw_bottom_right();
	unsigned int set_drawing_offset();
	unsigned int set_draw_mode();
	unsigned int set_texture_window();
	unsigned int set_mask_bit();
	unsigned int clear_cache();
	unsigned int copy_rectangle_from_cpu_to_vram();

	unsigned int mono_4_pt_opaque();
};