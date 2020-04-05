#include "DebugMenu.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Cpu.hpp"
#include "Gpu.hpp"
#include "MemoryMap.hpp"

#include <sstream>
#include <iomanip>

void DebugMenu::init(GLFWwindow* window, std::shared_ptr<Cpu> _cpu, std::shared_ptr<Gpu> _gpu, std::shared_ptr<Ram> _ram)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

	cpu = _cpu;
	gpu = _gpu;
	ram = _ram;
}

void DebugMenu::deinit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void DebugMenu::draw()
{
	// draw imgui debug menu
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	draw_cpu_menu();

	draw_gpu_menu();

	draw_controls_menu();

	draw_ram_menu();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugMenu::draw_cpu_menu()
{
	ImGui::Begin("CPU Registers");

	{
		std::stringstream current_pc_text;
		current_pc_text << "PC: 0x" << std::hex << std::setfill('0') << std::setw(8) << cpu->current_pc;
		ImGui::Text(current_pc_text.str().c_str());
	}

	{
		std::stringstream current_instr_text;
		current_instr_text << "Instr: 0x" << std::hex << std::setfill('0') << std::setw(8) << cpu->current_instruction;
		ImGui::Text(current_instr_text.str().c_str());
		ImGui::Separator();
	}

	{
		for (int idx = 0; idx < 32; idx++)
		{
			std::stringstream reg_text;
			reg_text << "R[" << idx << "]: 0x" << std::hex << std::setfill('0') << std::setw(8) << cpu->register_file.gp_registers[idx];
			ImGui::Text(reg_text.str().c_str());
		}
	}

	ImGui::End();
}

void DebugMenu::draw_gpu_menu()
{
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

void DebugMenu::draw_controls_menu()
{
	ImGui::Begin("Controls");

	{
		if (ImGui::Button(paused_requested ? "Start" : "Stop"))
		{
			paused_requested = !paused_requested;
		}
		ImGui::SameLine();
		if (ImGui::Button("Step"))
		{
			step_requested = !step_requested;
		}

		save_state_requested = ImGui::Button("Save state");
		ImGui::SameLine();
		load_state_requested = ImGui::Button("Load state");
	}

	ImGui::End();
}

void DebugMenu::draw_ram_menu()
{
	ImGui::Begin("RAM");

	static int address_of_interest = 0x00138d04;
	ImGui::InputInt("Address (hex)", &address_of_interest , 4, 100, ImGuiInputTextFlags_CharsHexadecimal);

	static bool pause_on_write = false;
	ImGui::Checkbox("Pause on write (roughly)", &pause_on_write);
	
	{
		static unsigned int prev_value = 0x0;
		unsigned int value = ram->load<unsigned int>(address_of_interest);

		std::stringstream text;
		text << "Word: 0x" << std::hex << std::setfill('0') << std::setw(8) << value;
		ImGui::Text(text.str().c_str());

		if (prev_value != value && pause_on_write == true)
		{
			paused_requested = true;
		}

		prev_value = value;
	}

	{
		std::stringstream text;
		text << "Halfword: 0x" << std::hex << std::setfill('0') << std::setw(4) << ram->load<unsigned short>(address_of_interest);
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		text << "Byte: 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)ram->load<unsigned char>(address_of_interest);
		ImGui::Text(text.str().c_str());
	}

	ImGui::End();
}