#include "DebugConsole.hpp"
#include <sstream>
#include <iostream>
#include "apg_console.h"

void DebugConsole::draw_in_category(menubar_category category)
{
	if (category == menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Console", &is_visible);
	}
}

void DebugConsole::draw_menu()
{
	if (is_visible == false) return;

	ImGui::Begin("Console");
	static char input_field[256] = "\0";
	static char output_buffer[1024] = "\0";

	ImGui::Text("Input");
	if (ImGui::InputText("##console_input", input_field, 255, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		apg_c_append_user_entered_text((std::string(input_field) + "\n").c_str());
		memset(input_field, 0, 256);
		apg_c_dump_to_buffer(output_buffer, 1024);
	}

	ImGui::Text("Output");
	ImGui::InputTextMultiline("##console_output", output_buffer, 1024, ImVec2(-1,-1), ImGuiInputTextFlags_ReadOnly);

	ImGui::End();
}

void DebugConsole::tick()
{
}
