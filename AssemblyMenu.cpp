#include "AssemblyMenu.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Psx.hpp"
#include "Cpu.hpp"
#include "Bus.hpp"
#include "MipsToString.hpp"

#include <sstream>
#include <iomanip>
#include <fstream>

AssemblyMenu::AssemblyMenu(std::shared_ptr<Psx> _psx) :
	DebugMenu(_psx)
{
	std::ifstream comment_log("comment_file.txt");
	if (comment_log.is_open())
	{
		std::string pc_value;
		std::string comment_value;
		while (std::getline(comment_log, pc_value) && std::getline(comment_log, comment_value))
		{
			char * buffer = new char[256];
			memset(buffer, 0, 256);
			// 254 to ensure we don't overrun the null termination
			strncat(buffer, comment_value.c_str(), 254);
			assembly_comment_buffer[std::stoul(pc_value)] = buffer;
		}
		comment_log.close();
	}
}

AssemblyMenu::~AssemblyMenu()
{
	std::ofstream comment_log("comment_file.txt");
	for (auto iter : assembly_comment_buffer)
	{
		if (comment_log.is_open() && strlen(iter.second) > 0)
		{
			comment_log << iter.first << std::endl;
			comment_log << iter.second << std::endl;
		}
		delete[] iter.second;
	}
	comment_log.close();
	assembly_comment_buffer.clear();
}

void AssemblyMenu::draw_in_category(DebugMenu::menubar_category category)
{
	if (category == DebugMenu::menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Assembly", &is_visible);
	}
}

void AssemblyMenu::draw_menu()
{
	if (is_visible == false) return;

	ImGui::Begin("Assembly");

	unsigned int pc = psx->cpu->current_pc;

	psx->bus->suppress_exceptions = true;

	for (int idx = -10; idx < 10; idx++)
	{
		std::stringstream asm_text;
		unsigned int pc = psx->cpu->current_pc + static_cast<unsigned int>(idx * 4);
		instruction_union instruction = psx->bus->get_word(pc);
		// current instruction
		if (pc == psx->cpu->current_pc)
		{
			asm_text << ">>";
		}
		// next instruction
		else if (pc == psx->cpu->next_pc)
		{
			asm_text << "->";
		}
		else
		{
			asm_text << "  ";
		}
		asm_text << "0x" << std::hex << std::setfill('0') << std::setw(8) << instruction.raw << "; " << MipsToString::instruction_to_string(instruction) << "\n";
		
		if (MipsToString::is_branch_or_jump(instruction))
		{
			static ImVec4 red = { 1, 0, 0, 1 };
			ImGui::TextColored(red, asm_text.str().c_str());
		}
		else
		{
			static ImVec4 yellow = { 1, 1, 0, 1 };
			ImGui::TextColored(yellow, asm_text.str().c_str());
		}
		
		
		ImGui::SameLine();

		char * buffer = assembly_comment_buffer[pc];
		if (buffer == nullptr)
		{
			buffer = new char[256];
			memset(buffer, 0, 256);
			assembly_comment_buffer[pc] = buffer;
		}

		ImGui::InputText((std::string("##") + std::to_string(idx)).c_str(), buffer, 256);
	}
	psx->bus->suppress_exceptions = false;

	ImGui::End();
}

void AssemblyMenu::tick()
{

}
