#include "CpuMenu.hpp"
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
}

void CpuMenu::draw_menu()
{
	if (is_visible == false) return;

	Cpu * cpu = Cpu::get_instance();

	ImGui::Begin("CPU Registers");

	{
		std::stringstream current_pc_text;
		current_pc_text << "PC: 0x" << std::hex << std::setfill('0') << std::setw(8) << cpu->current_pc;
		ImGui::Text(current_pc_text.str().c_str());
	}

	{
		std::stringstream current_instr_text;
		current_instr_text << "Instr: 0x" << std::hex << std::setfill('0') << std::setw(8) << cpu->current_instruction;
		ImGui::Text(current_instr_text.str().c_str());
		ImGui::Separator();
	}

	if (ImGui::TreeNode("Registers"))
	{	
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
			std::string reg_name = MipsToString::register_to_string(idx);
			int * reg_ref = reinterpret_cast<int*>(cpu->register_file.get_register_ref(idx));
			ImGui::InputInt(reg_name.c_str(), reg_ref, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Coprocessor 0"))
	{
		SystemControlCoprocessor * cop0 = SystemControlCoprocessor::get_instance();

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ0_VBLANK;
			bool irq = cop0->interrupt_status_register.IRQ0_VBLANK;
			text << "IRQ0_VBLANK: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ1_GPU;
			bool irq = cop0->interrupt_status_register.IRQ1_GPU;
			text << "IRQ1_GPU: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ2_CDROM;
			bool irq = cop0->interrupt_status_register.IRQ2_CDROM;
			text << "IRQ2_CDROM: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ3_DMA;
			bool irq = cop0->interrupt_status_register.IRQ3_DMA;
			text << "IRQ3_DMA: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ4_TMR0;
			bool irq = cop0->interrupt_status_register.IRQ4_TMR0;
			text << "IRQ4_TMR0: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ5_TMR1;
			bool irq = cop0->interrupt_status_register.IRQ5_TMR1;
			text << "IRQ5_TMR1: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ6_TMR2;
			bool irq = cop0->interrupt_status_register.IRQ6_TMR2;
			text << "IRQ6_TMR2: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ7_CTRL_MEM_CRD;
			bool irq = cop0->interrupt_status_register.IRQ7_CTRL_MEM_CRD;
			text << "IRQ7_CTRL_MEM_CRD: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ8_SIO;
			bool irq = cop0->interrupt_status_register.IRQ8_SIO;
			text << "IRQ8_SIO: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ9_SPU;
			bool irq = cop0->interrupt_status_register.IRQ9_SPU;
			text << "IRQ9_SPU: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			bool enabled = cop0->interrupt_mask_register.IRQ10_LIGHTPEN;
			bool irq = cop0->interrupt_status_register.IRQ10_LIGHTPEN;
			text << "IRQ10_LIGHTPEN: " << (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
			ImGui::Text(text.str().c_str());
		}

		ImGui::TreePop();
	}

	Dma * dma = Dma::get_instance();

	if (ImGui::TreeNode("DMA"))
	{
		ImGui::TreePop();
		DMA_interrupt_register reg = dma->interrupt_register;

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
