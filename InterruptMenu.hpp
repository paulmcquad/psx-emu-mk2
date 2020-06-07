#pragma once
#include "DebugMenuManager.hpp"
#include <deque>

class InterruptMenu : public DebugMenu
{
public:
	InterruptMenu(std::shared_ptr<Psx> _psx);

	virtual void draw_in_category(menubar_category category) final;
	virtual void draw_menu() final;
	virtual void tick() final;

private:
	bool is_visible = false;

	static const int MAX_VALUE_COUNT = 1000;
	static const int NUM_IRQS = 11;

	std::deque<float> i_stat_irqs[NUM_IRQS];
	std::deque<float> i_mask_irqs[NUM_IRQS];
	
};