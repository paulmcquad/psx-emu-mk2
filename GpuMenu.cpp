#include "GpuMenu.hpp"
#include "Psx.hpp"
#include "Gpu.hpp"

#include <sstream>
#include <iomanip>

void GpuMenu::draw_in_category(menubar_category category)
{
	if (category == menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Gpu", &is_visible);
	}
}

void GpuMenu::draw_menu()
{
	if (is_visible == false) return;

	Gpu * gpu = Gpu::get_instance();

	ImGui::Begin("GPU");

	{
		std::stringstream status_text;
		status_text << "Status Register: 0x" << std::hex << std::setfill('0') << std::setw(8) << gpu->gpu_status.int_value;
		ImGui::Text(status_text.str().c_str());

		{
			std::stringstream text;
			text << "Drawing offsets: " << gpu->x_offset << " " << gpu->y_offset;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Video Mode: " << (gpu->gpu_status.video_mode == 0 ? "NTSC" : "PAL");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Vertical Interface: " << gpu->gpu_status.v_interlace;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			// 0 = Enabled
			text << "Display Enable: " << (gpu->gpu_status.display_enable == 0);
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Interrupt Request: " << gpu->gpu_status.irq_request;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Ready DMA block: " << gpu->gpu_status.ready_dma;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Ready CMD Word: " << gpu->gpu_status.ready_cmd_word;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Ready VRAM to CPU: " << gpu->gpu_status.ready_vram_to_cpu;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "DMA Direction: ";
			switch (gpu->gpu_status.dma_direction)
			{
			case 0:
				text << "Off";
				break;
			case 1:
				text << "?";
				break;
			case 2:
				text << "CPUtoGP0";
				break;
			case 3:
				text << "GPUREADtoCPU";
				break;
			}
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Drawing: " << (gpu->gpu_status.even_odd ? "Odd" : "Even/Vblank");
			ImGui::Text(text.str().c_str());
		}

		// todo add more
	}

	ImGui::End();
}

void GpuMenu::tick()
{
}
