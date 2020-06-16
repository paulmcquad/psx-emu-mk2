#pragma once
#include "DebugMenuManager.hpp"

class CdromMenu : public DebugMenu
{
public:
	virtual void draw_in_category(menubar_category category) final;
	virtual void draw_menu() final;
	virtual void tick() final;
private:
	bool is_visible = false;
};