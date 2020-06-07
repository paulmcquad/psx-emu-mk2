#include "InterruptMenu.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "SystemControlCoprocessor.hpp"
#include "Psx.hpp"
#include "Cpu.hpp"

InterruptMenu::InterruptMenu(std::shared_ptr<Psx> _psx) : DebugMenu(_psx)
{
	
}

void InterruptMenu::draw_in_category(menubar_category category)
{
	if (category == menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Interrupts", &is_visible);
	}
}

void InterruptMenu::draw_menu()
{
	if (is_visible == false)
	{
		return;
	}

	ImGui::Begin("Interrupt");
	ImGui::Text("I_STAT");
	for (int idx = 0; idx < NUM_IRQS; idx++)
	{
		ImGui::PlotLines(std::to_string(idx).c_str(), 
			[](void *data, int idx) {
			std::deque<float> * values = static_cast<std::deque<float>*>(data);
			if (static_cast<unsigned int>(idx) < values->size())
			{
				return (*values)[idx];
			}
			else
			{
				return 0.f;
			}
			},
			reinterpret_cast<void*>(&i_stat_irqs[idx]), MAX_VALUE_COUNT, 0, 0, 0.f, 1.f);
	}

	ImGui::Text("I_MASK");
	for (int idx = 0; idx < NUM_IRQS; idx++)
	{
		ImGui::PlotLines(std::to_string(idx).c_str(),
			[](void *data, int idx) {
			std::deque<float> * values = static_cast<std::deque<float>*>(data);
			if (static_cast<unsigned int>(idx) < values->size())
			{
				return (*values)[idx];
			}
			else
			{
				return 0.f;
			}
			},
			reinterpret_cast<void*>(&i_mask_irqs[idx]), MAX_VALUE_COUNT, 0, 0, 0.f, 1.f);
	}
	ImGui::End();
}

void InterruptMenu::tick()
{
	if (is_visible == false)
	{
		return;
	}

	system_control::interrupt_register i_stat = psx->cpu->cop0->interrupt_status_register;
	system_control::interrupt_register i_mask = psx->cpu->cop0->interrupt_mask_register;

	unsigned int i_stat_bits = i_stat.IRQ_BITS;
	unsigned int i_mask_bits = i_mask.IRQ_BITS;
	for (int idx = 0; idx < NUM_IRQS; idx++)
	{
		bool i_stat_for_idx = i_stat_bits & 0x1;
		bool i_mask_for_idx = i_mask_bits & 0x1;

		i_stat_irqs[idx].push_back(i_stat_for_idx ? 1.f : 0.f);
		i_mask_irqs[idx].push_back(i_mask_for_idx ? 1.f : 0.f);

		if (i_stat_irqs[idx].size() > MAX_VALUE_COUNT)
		{
			// can pop both as they will have the exact same size
			i_stat_irqs[idx].pop_front();
			i_mask_irqs[idx].pop_front();
		}

		// shift to next irq bit to check
		i_stat_bits >>= 1;
		i_mask_bits >>= 1;
	}
}
