#pragma once
#include "DebugMenuManager.hpp"

class AssemblyMenu : public DebugMenu
{
public:
	AssemblyMenu();
	~AssemblyMenu();

	virtual void draw_in_category(menubar_category category) final;
	virtual void draw_menu() final;
	virtual void tick() final;

private:
	bool is_visible = false;
	std::map<unsigned int, char *> assembly_comment_buffer;
};