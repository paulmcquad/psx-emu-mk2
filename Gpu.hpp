#pragma once
#include <vector>
#include <deque>
#include "Dma.hpp"

constexpr unsigned int FRAME_WIDTH = 1024;
constexpr unsigned int FRAME_HEIGHT = 512;
constexpr unsigned int BYTES_PER_PIXEL = 2;
constexpr unsigned int VRAM_SIZE = FRAME_WIDTH * FRAME_HEIGHT * BYTES_PER_PIXEL;

// don't know why this isn't defined
constexpr unsigned int GL_RGB565 = 0x8D62;

class Gpu : public DMA_interface
{
public:

	void init();
	void reset();
	void tick();

	virtual void sync_mode_request(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;
	virtual void sync_mode_linked_list(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;

	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
	} GP0_send;

	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
	} GP1_send;

	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
	} GPUREAD;

	union
	{
		unsigned int int_value = 0;
		struct
		{
			unsigned char byte_value[4];
		};
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
	} GPUSTAT;

	std::vector<unsigned char> video_ram;
	std::deque<unsigned int> commands;
	unsigned int width = FRAME_WIDTH;
	unsigned int height = FRAME_HEIGHT;

private:
	void draw_static();
};