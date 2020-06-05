#include "CpuMenu.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Psx.hpp"
#include "Cpu.hpp"
#include "Dma.hpp"
#include "SystemControlCoprocessor.hpp"
#include "MipsToString.hpp"
#include <sstream>
#include <iomanip>

void CpuMenu::draw_in_category(menubar_category category)
{
	if (category == menubar_category::VIEW)
	{
		ImGui::Checkbox("Show Cpu", &is_visible);
	}
	else if (category == menubar_category::OPTIONS)
	{
		ImGui::Checkbox("Break on register load delay overwritten", &psx->cpu->register_file.break_on_overwrite);
	}
}

void CpuMenu::draw_menu()
{
	if (is_visible == false) return;

	ImGui::Begin("CPU Registers");

	{
		std::stringstream current_pc_text;
		current_pc_text << "PC: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->cpu->current_pc;
		ImGui::Text(current_pc_text.str().c_str());
	}

	{
		std::stringstream current_instr_text;
		current_instr_text << "Instr: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->cpu->current_instruction;
		ImGui::Text(current_instr_text.str().c_str());
		ImGui::Separator();
	}

	if (ImGui::TreeNode("Registers"))
	{	
		ImGui::InputInt("Only pause on" , &psx->cpu->register_file.pause_on_value, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::Checkbox("Only pause on specific value", &psx->cpu->register_file.only_pause_on_value);
		ImGui::Separator();
		for (int idx = 0; idx < 32; idx++)
		{
			// add register names
			std::stringstream reg_text;
			if (idx == 2)
			{
				ImGui::Text("Results");
			}
			else if (idx == 4)
			{
				ImGui::Text("Arguments");
			}
			else if (idx == 8 || idx == 24)
			{
				ImGui::Text("Temps - not saved");
			}
			else if (idx == 16)
			{
				ImGui::Text("Saved");
			}
			else if (idx == 26)
			{
				ImGui::Text("Kernel");
			}
			else if (idx == 28)
			{
				ImGui::Text("Pointers");
			}

			bool overwrite_during_load_delay_detected = psx->cpu->register_file.overwrites[idx] != 0;
			if (overwrite_during_load_delay_detected)
			{
				ImGui::Separator();
				static ImVec4 red = { 1, 0, 0, 1 };
				ImGui::TextColored(red, std::to_string(psx->cpu->register_file.overwrites[idx]).c_str());
				ImGui::SameLine();
			}

			std::string reg_name = MipsToString::register_to_string(idx);
			ImGui::Checkbox(std::string("##" + reg_name).c_str(), &psx->cpu->register_file.break_on_change[idx]);
			ImGui::SameLine();
			// register contents
			int * reg_ref = reinterpret_cast<int*>(psx->cpu->register_file.get_register_ref(idx));
			ImGui::InputInt(reg_name.c_str(), reg_ref, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);

			if (overwrite_during_load_delay_detected)
			{
				ImGui::Separator();
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Coprocessor 0"))
	{
		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ0_VBLANK;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ0_VBLANK;
			text << "IRQ0_VBLANK: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ1_GPU;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ1_GPU;
			text << "IRQ1_GPU: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ2_CDROM;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ2_CDROM;
			text << "IRQ2_CDROM: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ3_DMA;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ3_DMA;
			text << "IRQ3_DMA: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ4_TMR0;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ4_TMR0;
			text << "IRQ4_TMR0: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ5_TMR1;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ5_TMR1;
			text << "IRQ5_TMR1: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ6_TMR2;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ6_TMR2;
			text << "IRQ6_TMR2: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ7_CTRL_MEM_CRD;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ7_CTRL_MEM_CRD;
			text << "IRQ7_CTRL_MEM_CRD: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ8_SIO;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ8_SIO;
			text << "IRQ8_SIO: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ9_SPU;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ9_SPU;
			text << "IRQ9_SPU: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ10_LIGHTPEN;
			bool irq = psx->cpu->cop0->interrupt_status_register.IRQ10_LIGHTPEN;
			text << "IRQ10_LIGHTPEN: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("DMA"))
	{
		ImGui::TreePop();
		DMA_interrupt_register reg = psx->dma->interrupt_register;

		{
			std::stringstream text;
			text << "IRQ Master Flag: " << (reg.irq_master_flag ? "Enabled " : "Disabled ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "IRQ Master Enable: " << (reg.irq_master_enable ? "Enabled " : "Disabled ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Force IRQ: " << (reg.force_irq ? "Force " : "None ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "IRQ Enable: " << std::hex << reg.irq_enable;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "IRQ Flags: " << std::hex << reg.irq_flags;
			ImGui::Text(text.str().c_str());
		}
	}

	ImGui::End();
}

void CpuMenu::tick()
{
}
