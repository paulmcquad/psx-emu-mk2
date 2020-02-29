#include "DebugMenu.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Cpu.hpp"
#include "Gpu.hpp"

#include <sstream>
#include <iomanip>

void DebugMenu::init(GLFWwindow* window, std::shared_ptr<Cpu> _cpu, std::shared_ptr<Gpu> _gpu)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

	cpu = _cpu;
	gpu = _gpu;
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

	draw_overview_menu();

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
			std::stringstream offset_text;
			offset_text << "Drawing offsets: " << gpu->x_offset << " " << gpu->y_offset;
			ImGui::Text(offset_text.str().c_str());
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

void DebugMenu::draw_overview_menu()
{

}