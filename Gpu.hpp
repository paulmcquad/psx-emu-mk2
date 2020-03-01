#pragma once
#include <vector>
#include <deque>
#include <unordered_map>
#include <glm/fwd.hpp>
#include "Dma.hpp"

constexpr unsigned int FRAME_WIDTH = 1024;
constexpr unsigned int FRAME_HEIGHT = 512;
constexpr unsigned int VRAM_SIZE = FRAME_WIDTH * FRAME_HEIGHT;

enum class gp0_commands : unsigned char;
enum class gp1_commands : unsigned char;

class Gpu : public DMA_interface
{
public:

	void init();
	void reset();
	void tick();
	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

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

	union gpu_status_union
	{
		unsigned int int_value;
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

		gpu_status_union() = default;
		gpu_status_union(unsigned int val)
		{
			int_value = val;
		}
	} gpu_status;

	std::vector<unsigned short> video_ram;
	// apparently the psx has a 16 word FIFO queue, i'm going to work under the assumption
	// that I can ignore that
	std::deque<unsigned int> gp0_command_queue;
	unsigned int width = FRAME_WIDTH;
	unsigned int height = FRAME_HEIGHT;

	int x_offset = 0;
	int y_offset = 0;

	bool logging_enabled = false;

private:
	std::unordered_map<gp0_commands, unsigned int (Gpu::*)()> gp0_command_map;
	std::unordered_map<gp1_commands, void (Gpu::*)(unsigned int)> gp1_command_map;

	void execute_gp0_commands();
	void add_gp0_command(unsigned int command, bool via_dma);
	void execute_gp1_command(unsigned int command);

	void draw_triangle(glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2, glm::u8vec3 rgb);
	void draw_pixel(glm::ivec2 v, glm::u8vec3 rgb);

	// GP0 commands
	unsigned int nop();
	unsigned int set_draw_top_left();
	unsigned int set_draw_bottom_right();
	unsigned int set_drawing_offset();
	unsigned int set_draw_mode();
	unsigned int set_texture_window();
	unsigned int set_mask_bit();
	unsigned int clear_cache();

	unsigned int copy_rectangle_from_cpu_to_vram();
	unsigned int copy_rectangle_from_vram_to_cpu();

	unsigned int shaded_3_pt_opaque();
	unsigned int mono_4_pt_opaque();
	unsigned int shaded_4_pt_opaque();
	unsigned int tex_4_pt_opaque_blend();

	// GP1 commands]
	void reset_gpu(unsigned int command);
	void reset_command_buffer(unsigned int command);
	void ack_gpu_interrupt(unsigned int command);
	void display_enable(unsigned int command);
	void dma_direction(unsigned int command);
	void start_display_area(unsigned int command);
	void horizontal_display_range(unsigned int command);
	void vertical_display_range(unsigned int command);
	void display_mode(unsigned int command);
	void get_gpu_info(unsigned int command);
	void new_texture_disable(unsigned int command);
	void special_texture_disable(unsigned int command);
};