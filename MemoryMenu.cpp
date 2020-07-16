#include "MemoryMenu.hpp"

#include "Psx.hpp"
#include "Cpu.hpp"
#include "Bus.hpp"
#include "CacheControl.hpp"

#include <sstream>
#include <iomanip>

void MemoryMenu::draw_in_category(menubar_category category)
{
	if (category == menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Memory", &is_visible);
	}
}

void MemoryMenu::draw_menu()
{
	if (is_visible == false) return;

	Bus * bus = Bus::get_instance();
	ImGui::Begin("Bus");

	static int address_of_interest = 0x0;
	ImGui::InputInt("Address", &address_of_interest, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);

	try
	{
		std::stringstream text;
		text << "Word: 0x" << std::hex << std::setfill('0') << std::setw(8) << bus->get_word(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Word access not supported");
	}

	try
	{
		std::stringstream text;
		text << "Halfword: 0x" << std::hex << std::setfill('0') << std::setw(4) << bus->get_halfword(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Halfword access not supported");
	}

	try
	{
		std::stringstream text;
		text << "Byte: 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)bus->get_byte(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Byte access not supported");
	}

	static int new_value = 0x0;
	ImGui::NewLine();
	ImGui::InputInt("New Value", &new_value, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	if (ImGui::Button("Apply as Word"))
	{
		bus->set_word(address_of_interest, new_value);
	}

	if (ImGui::Button("Apply as Halfword"))
	{
		bus->set_halfword(address_of_interest, new_value);
	}

	if (ImGui::Button("Apply as Byte"))
	{
		bus->set_byte(address_of_interest, new_value);
	}

	ImGui::End();

	ImGui::Begin("Cache Control");
	ImGui::Text(CacheControl::get_instance()->cache_control_register.scratch_pad_enable1 ?
		"Scratchpad Enable 1: Enabled" : "Scratchpad Enable 1: Disabled");
	ImGui::Text(CacheControl::get_instance()->cache_control_register.scratch_pad_enable2 ?
		"Scratchpad Enable 2: Enabled" : "Scratchpad Enable 2: Disabled");
	ImGui::Text(CacheControl::get_instance()->cache_control_register.crash ?
		"Crash: Enabled" : "Crash: Disabled");
	ImGui::Text(CacheControl::get_instance()->cache_control_register.code_cache_enable ?
		"Code Cache: Enabled" : "Code Cache: Disabled");
	ImGui::End();
}

void MemoryMenu::tick()
{
}
