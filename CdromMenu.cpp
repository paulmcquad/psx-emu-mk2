#include "CdromMenu.hpp"

#include "Psx.hpp"
#include "Cdrom.hpp"

#include <sstream>
#include <iomanip>

void CdromMenu::draw_in_category(menubar_category category)
{
	if (category == menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Cdrom", &is_visible);
	}
}

void CdromMenu::draw_menu()
{
	if (is_visible == false) return;

	ImGui::Begin("Cdrom");

	unsigned char cdrom_status_register = Cdrom::get_instance()->get_byte(Cdrom::STATUS_REGISTER);

	std::stringstream text;
	text << "Status Register: 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)cdrom_status_register;
	ImGui::Text(text.str().c_str());

	ImGui::End();
}

void CdromMenu::tick()
{
}
