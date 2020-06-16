#pragma once
#include <vector>
#include <deque>
#include <unordered_map>
#include <glm/fwd.hpp>
#include "Fifo.hpp"
#include "InstructionTypes.hpp"
#include "Dma.hpp"
#include "Bus.hpp"

enum class gp0_commands : unsigned char;
enum class gp1_commands : unsigned char;

class Gpu : public DMA_interface, public Bus::BusDevice
{
public:
	static const unsigned int FRAME_WIDTH = 1024;
	static const unsigned int FRAME_HEIGHT = 512;
	
	virtual bus_device_type get_bus_device_type() final { return bus_device_type::GPU; }

	virtual bool is_peripheral() final { return true; }

	virtual bool is_address_for_device(unsigned int address) final;

	// AFAIK the GPU is only accessed with full word accesses
	virtual unsigned int get_word(unsigned int address) final;
	virtual void set_word(unsigned int address, unsigned int value) final;

	~Gpu();
	void init();
	void reset();
	void tick();
	void save_state(std::stringstream& file, bool ignore_vram = false);
	void load_state(std::stringstream& file, bool ignore_vram = false);

	virtual void sync_mode_request(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;
	virtual void sync_mode_linked_list(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;

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

	Fifo<unsigned int> * gp0_fifo = nullptr;
	unsigned short * video_ram = nullptr;

	unsigned int width = FRAME_WIDTH;
	unsigned int height = FRAME_HEIGHT;

	int x_offset = 0;
	int y_offset = 0;

	unsigned int draw_area_min_x = 0;
	unsigned int draw_area_min_y = 0;
	unsigned int draw_area_max_x = 0;
	unsigned int draw_area_max_y = 0;

	static Gpu * get_instance();

private:
	static const unsigned int VRAM_SIZE = FRAME_WIDTH * FRAME_HEIGHT;
	static const unsigned int GP0_FIFO_SIZE = 16;

	static const unsigned int GPU_SIZE = 8;
	static const unsigned int GPU_START = 0x1F801810;
	static const unsigned int GPU_END = GPU_START + GPU_SIZE;

	static const unsigned int GP0_Send_GPUREAD = 0x1F801810;
	static const unsigned int GP1_Send_GPUSTAT = 0x1f801814;

	// copying variables
	gp_command copy_to_gpu_dest_coord = 0x0;
	gp_command copy_to_gpu_current_coord = 0x0;
	gp_command copy_to_gpu_width_height = 0x0;
	unsigned int num_words_to_copy_to_gpu = 0;

	gp_command copy_to_cpu_src_coord = 0x0;
	gp_command copy_to_cpu_current_coord = 0x0;
	gp_command copy_to_cpu_width_height = 0x0;
	unsigned int num_words_to_copy_to_cpu = 0;

	void execute_gp0_commands();
	void add_gp0_command(gp_command command, bool via_dma);
	void execute_gp1_command(gp_command command);

	void draw_triangle(glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2, glm::u8vec3 rgb0, glm::u8vec3 rgb1, glm::u8vec3 rgb2);
	void draw_rectangle(glm::ivec2 top_left, glm::ivec2 width_height, glm::u8vec3 rgb);
	void draw_pixel(glm::ivec2 v, glm::u8vec3 rgb, bool ignore_draw_offsets = false);

	void copy_next_pixel_to_framebuffer(unsigned int short pixel_data);
	unsigned short copy_next_pixel_from_framebuffer();

	glm::vec3 calc_barycentric(glm::ivec2 pos, glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2);

	// GP0 commands
	bool set_draw_top_left();
	bool set_draw_bottom_right();
	bool set_drawing_offset();
	bool set_draw_mode();
	bool set_texture_window();
	bool set_mask_bit();
	bool clear_cache();

	bool copy_rectangle_from_cpu_to_vram();
	bool copy_rectangle_from_vram_to_cpu();

	bool shaded_3_pt_opaque();
	bool mono_4_pt_opaque();
	bool shaded_4_pt_opaque();
	bool tex_4_pt_opaque_blend();

	bool fill_rect();
};