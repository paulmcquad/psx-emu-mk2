#pragma once
#include "DebugMenuManager.hpp"

class MemoryMenu : public DebugMenu
{
public:
	MemoryMenu(std::shared_ptr<Psx> _psx) : DebugMenu(_psx) {};

	virtual void draw_in_category(menubar_category category) final;

	virtual void draw_menu() final;

	virtual void tick() final;

private:
	bool is_visible = false;

};