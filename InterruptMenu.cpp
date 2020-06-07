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
	for (int idx = 0; idx < NUM_IRQS; idx++)
	{
		i_stat_irqs[idx] = new CircularBuffer<float>(MAX_VALUE_COUNT);
		i_mask_irqs[idx] = new CircularBuffer<float>(MAX_VALUE_COUNT);
	}
}

InterruptMenu::~InterruptMenu()
{
	for (int idx = 0; idx < NUM_IRQS; idx++)
	{
		delete i_stat_irqs[idx];
		delete i_mask_irqs[idx];
	}
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
	ImPlot::SetNextPlotLimits(0, MAX_VALUE_COUNT, -0.1f, 1.1f, ImGuiCond_Always);
	if (ImPlot::BeginPlot("I_STAT", "ticks", nullptr, ImVec2(0, 0)))
	{
		for (int idx = 0; idx < NUM_IRQS; idx++)
		{
			ImPlot::PlotLine(labels[idx],
				[](void *data, int _idx) {
				CircularBuffer<float> * values = static_cast<CircularBuffer<float>*>(data);
				ImPlotPoint result;
				result.x = _idx;
				result.y = values->get(_idx);

				return result;
			},
				reinterpret_cast<void*>(i_stat_irqs[idx]), MAX_VALUE_COUNT);
		}
		ImPlot::EndPlot();
	}
	ImGui::SameLine();
	ImPlot::SetNextPlotLimits(0, MAX_VALUE_COUNT, -0.1f, 1.1f, ImGuiCond_Always);
	if (ImPlot::BeginPlot("I_MASK", "ticks", nullptr, ImVec2(0,0)))
	{
		for (int idx = 0; idx < NUM_IRQS; idx++)
		{
			ImPlot::PlotLine(labels[idx],
				[](void *data, int _idx) {
				CircularBuffer<float> * values = static_cast<CircularBuffer<float>*>(data);
				ImPlotPoint result;
				result.x = _idx;
				result.y = values->get(_idx);

				return result;
			},
			reinterpret_cast<void*>(i_mask_irqs[idx]), MAX_VALUE_COUNT);
		}
		ImPlot::EndPlot();
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

		i_stat_irqs[idx]->push(i_stat_for_idx ? 1.f : 0.f);
		i_mask_irqs[idx]->push(i_mask_for_idx ? 1.f : 0.f);

		// shift to next irq bit to check
		i_stat_bits >>= 1;
		i_mask_bits >>= 1;
	}
}
