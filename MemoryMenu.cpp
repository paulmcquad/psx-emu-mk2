#include "MemoryMenu.hpp"

#include "Psx.hpp"
#include "Cpu.hpp"
#include "Bus.hpp"

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

	ImGui::Begin("Bus");

	psx->bus->suppress_exceptions = true;

	static int address_of_interest = 0x0;
	ImGui::InputInt("Address", &address_of_interest, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);

	bool save_enable_pause_state = psx->bus->enable_pause_on_address_access;
	psx->bus->enable_pause_on_address_access = false;
	try
	{
		std::stringstream text;
		text << "Word: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->bus->get_word(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Word access not supported");
	}

	try
	{
		std::stringstream text;
		text << "Halfword: 0x" << std::hex << std::setfill('0') << std::setw(4) << psx->bus->get_halfword(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Halfword access not supported");
	}

	try
	{
		std::stringstream text;
		text << "Byte: 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)psx->bus->get_byte(address_of_interest);
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
		psx->bus->set_word(address_of_interest, new_value);
	}

	if (ImGui::Button("Apply as Halfword"))
	{
		psx->bus->set_halfword(address_of_interest, new_value);
	}

	if (ImGui::Button("Apply as Byte"))
	{
		psx->bus->set_byte(address_of_interest, new_value);
	}

	psx->bus->enable_pause_on_address_access = save_enable_pause_state;

	if (ImGui::Button(psx->bus->enable_pause_on_address_access ? "Disable pause on access" : "Enable pause on access"))
	{
		psx->bus->enable_pause_on_address_access = !psx->bus->enable_pause_on_address_access;
		psx->bus->address_to_pause_on = address_of_interest;
	}

	psx->bus->suppress_exceptions = false;

	ImGui::End();
}

void MemoryMenu::tick()
{
}
