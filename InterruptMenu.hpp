#pragma once
#include "DebugMenuManager.hpp"
#include "CircularBuffer.hpp"

class InterruptMenu : public DebugMenu
{
public:
	InterruptMenu();
	virtual ~InterruptMenu();

	virtual void draw_in_category(menubar_category category) final;
	virtual void draw_menu() final;
	virtual void tick() final;

private:
	bool is_visible = false;

	static const int MAX_VALUE_COUNT = 1000;
	static const int NUM_IRQS = 11;

	CircularBuffer<float>* i_stat_irqs[NUM_IRQS];
	CircularBuffer<float>* i_mask_irqs[NUM_IRQS];

	char * labels[NUM_IRQS] = { "IRQ 0", "IRQ 1", "IRQ 2", "IRQ 3", "IRQ 4", "IRQ 5",
							    "IRQ 6", "IRQ 7", "IRQ 8", "IRQ 9", "IRQ 10"};
};