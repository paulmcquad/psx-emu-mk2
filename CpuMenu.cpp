#include "CpuMenu.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Psx.hpp"
#include "Cpu.hpp"
#include "MipsToString.hpp"
#include <sstream>
#include <iomanip>

void CpuMenu::draw_in_category(menubar_category category)
{
	if (category == menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Cpu", &is_visible);
	}
}

void CpuMenu::draw_menu()
{
	if (is_visible == false) return;

	ImGui::Begin("CPU Registers");

	{
		std::stringstream current_pc_text;
		current_pc_text << "PC: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->cpu->current_pc;
		ImGui::Text(current_pc_text.str().c_str());
	}

	{
		std::stringstream current_instr_text;
		current_instr_text << "Instr: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->cpu->current_instruction;
		ImGui::Text(current_instr_text.str().c_str());
		ImGui::Separator();
	}

	if (ImGui::TreeNode("Registers"))
	{
		for (int idx = 0; idx < 32; idx++)
		{
			// add register names
			std::stringstream reg_text;
			if (idx == 2)
			{
				ImGui::Text("Results");
			}
			else if (idx == 4)
			{
				ImGui::Text("Arguments");
			}
			else if (idx == 8 || idx == 24)
			{
				ImGui::Text("Temps - not saved");
			}
			else if (idx == 16)
			{
				ImGui::Text("Saved");
			}
			else if (idx == 26)
			{
				ImGui::Text("Kernel");
			}
			else if (idx == 28)
			{
				ImGui::Text("Pointers");
			}

			// register contents
			std::string reg_name = MipsToString::register_to_string(idx);
			int * reg_ref = reinterpret_cast<int*>(psx->cpu->register_file.get_register_ref(idx));
			ImGui::InputInt(reg_name.c_str(), reg_ref, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void CpuMenu::tick()
{
}
