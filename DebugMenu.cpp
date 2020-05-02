#include "DebugMenu.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Ram.hpp"
#include "IOPorts.hpp"

#include <sstream>
#include <iomanip>

void DebugMenu::init(GLFWwindow* window, std::shared_ptr<Cpu> _cpu, std::shared_ptr<Gpu> _gpu, std::shared_ptr<Ram> _ram, std::shared_ptr<IOPorts> _ioports)
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
	io_ports = _ioports;
}

void DebugMenu::uninit()
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

	draw_interrupt_menu();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugMenu::draw_cpu_menu()
{
	ImGui::Begin("CPU Registers");

	{
		std::stringstream current_ticks_text;
		current_ticks_text << "Ticks per frame: " << std::dec << ticks_per_frame;
		ImGui::Text(current_ticks_text.str().c_str());
	}

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
			reg_text << "R[" << idx << "]: 0x" << std::hex << std::setfill('0') << std::setw(8) << cpu->register_file.get_register(idx);
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

void DebugMenu::draw_interrupt_menu()
{
	ImGui::Begin("Interrupts");

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ0_VBLANK;
		bool irq = io_ports->interrupt_status_register.IRQ0_VBLANK;
		text << "IRQ0_VBLANK: " <<  (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ1_GPU;
		bool irq = io_ports->interrupt_status_register.IRQ1_GPU;
		text << "IRQ1_GPU: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ2_CDROM;
		bool irq = io_ports->interrupt_status_register.IRQ2_CDROM;
		text << "IRQ2_CDROM: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ3_DMA;
		bool irq = io_ports->interrupt_status_register.IRQ3_DMA;
		text << "IRQ3_DMA: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ4_TMR0;
		bool irq = io_ports->interrupt_status_register.IRQ4_TMR0;
		text << "IRQ4_TMR0: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ5_TMR1;
		bool irq = io_ports->interrupt_status_register.IRQ5_TMR1;
		text << "IRQ5_TMR1: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ6_TMR2;
		bool irq = io_ports->interrupt_status_register.IRQ6_TMR2;
		text << "IRQ6_TMR2: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ7_CTRL_MEM_CRD;
		bool irq = io_ports->interrupt_status_register.IRQ7_CTRL_MEM_CRD;
		text << "IRQ7_CTRL_MEM_CRD: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ8_SIO;
		bool irq = io_ports->interrupt_status_register.IRQ8_SIO;
		text << "IRQ8_SIO: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ9_SPU;
		bool irq = io_ports->interrupt_status_register.IRQ9_SPU;
		text << "IRQ9_SPU: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	{
		std::stringstream text;
		bool enabled = io_ports->interrupt_mask_register.IRQ10_LIGHTPEN;
		bool irq = io_ports->interrupt_status_register.IRQ10_LIGHTPEN;
		text << "IRQ10_LIGHTPEN: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
		ImGui::Text(text.str().c_str());
	}

	ImGui::End();
}