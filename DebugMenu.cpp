#include "DebugMenu.hpp"

#include <MipsToString.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Psx.hpp"
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Bus.hpp"
#include "SystemControlCoprocessor.hpp"

#include <sstream>
#include <iomanip>
#include <fstream>

void DebugMenu::init(GLFWwindow* window, std::shared_ptr<Psx> _psx)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

	psx = _psx;

	std::ifstream comment_log("comment_file.txt");
	if (comment_log.is_open())
	{
		std::string pc_value;
		std::string comment_value;
		while (std::getline(comment_log, pc_value) && std::getline(comment_log, comment_value))
		{
			char * buffer = new char[256];
			memset(buffer, 0, 256);
			// 254 to ensure we don't overrun the null termination
			strncat(buffer, comment_value.c_str(), 254);
			assembly_comment_buffer[std::stoul(pc_value)] = buffer;
		}
		comment_log.close();
	}
}

void DebugMenu::uninit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	std::ofstream comment_log("comment_file.txt");
	for (auto iter : assembly_comment_buffer)
	{
		if (comment_log.is_open() && strlen(iter.second) > 0)
		{
			comment_log << iter.first << std::endl;
			comment_log << iter.second << std::endl;
		}
		delete[] iter.second;
	}
	comment_log.close();
	assembly_comment_buffer.clear();

	for (auto iter : backward_states)
	{
		delete iter;
	}
	backward_states.clear();
	
}

void DebugMenu::draw()
{
	// draw imgui debug menu
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	draw_main_menu();

	draw_cpu_menu();

	draw_gpu_menu();

	draw_assembly_menu();

	draw_controls_menu();

	draw_interrupt_menu();

	draw_bus_menu();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugMenu::tick()
{
	if (recording_states)
	{
		if (backward_states.size() > MAX_BACKWARDS_STATES_SAVED)
		{
			delete backward_states.front();
			backward_states.pop_front();
		}

		std::stringstream * state = new std::stringstream();
		psx->save_state(*state);
		backward_states.push_back(state);
	}
}

void DebugMenu::draw_main_menu()
{
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save state")) { save_state_requested = true;  }
		if (ImGui::MenuItem("Load state")) { load_state_requested = true; }

		ImGui::EndMenu();
	}

	// step backwards
	if (ImGui::MenuItem("<-", nullptr, nullptr, backward_states.empty() == false)) {
		std::stringstream * state = backward_states.back();
		psx->load_state(*state);
		delete state;
		backward_states.pop_back();
	}

	// stop start
	if (ImGui::MenuItem(paused_requested ? "|>" : "||")) { paused_requested = !paused_requested; }

	// recording to enable backward steps
	if (ImGui::MenuItem(recording_states ? "[]" : "()")) { recording_states = !recording_states; }

	// step forwards
	if (ImGui::MenuItem("->")) { step_forward_requested = true; }

	ImGui::EndMainMenuBar();
}

void DebugMenu::draw_cpu_menu()
{
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

			if (idx == 0)
			{
				reg_text << "ZR-";
			}
			else if (idx == 1)
			{
				reg_text << "AT-";
			}
			else if (idx >= 2 && idx <= 3)
			{
				reg_text << "V" << idx - 2 << "-";
			}
			else if (idx >= 4 && idx <= 7)
			{
				reg_text << "A" << idx - 4 << "-";
			}
			else if (idx >= 8 && idx <= 15)
			{
				reg_text << "T" << idx - 8 << "-";
			}
			else if (idx >= 16 && idx <= 23)
			{
				reg_text << "S" << idx - 16 << "-";
			}
			else if (idx >= 24 && idx <= 25)
			{
				// starts at T8 now
				reg_text << "T" << idx - 16  << "-";
			}
			else if (idx >= 26 && idx <= 27)
			{
				reg_text << "K" << idx - 26 << "-";
			}
			else if (idx == 28 )
			{
				reg_text << "GP-";
			}
			else if (idx == 29)
			{
				reg_text << "SP-";
			}
			else if (idx == 30)
			{
				reg_text << "FP-";
			}
			else if (idx == 31)
			{
				reg_text << "RA-";
			}

			// register contents
			reg_text << "R[" << idx << "]: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->cpu->register_file.get_register(idx);
			ImGui::Text(reg_text.str().c_str());
		}

		static int register_of_interest = 0;
		ImGui::InputInt("Register:", &register_of_interest, 1, 1);
		if (register_of_interest < 0)
		{
			register_of_interest = 0;
		}
		else if (register_of_interest > 31)
		{
			register_of_interest = 31;
		}

		static int new_value = 0x0;
		ImGui::InputInt("New Register Value", &new_value, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
		if (ImGui::Button("Apply"))
		{
			psx->cpu->register_file.set_register(register_of_interest, new_value);
		}
	}

	ImGui::End();
}

void DebugMenu::draw_gpu_menu()
{
	ImGui::Begin("GPU");

	{
		std::stringstream status_text;
		status_text << "Status Register: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->gpu->gpu_status.int_value;
		ImGui::Text(status_text.str().c_str());

		{
			std::stringstream text;
			text << "Drawing offsets: " << psx->gpu->x_offset << " " << psx->gpu->y_offset;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Video Mode: " << (psx->gpu->gpu_status.video_mode == 0 ? "NTSC" : "PAL");
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Vertical Interface: " << psx->gpu->gpu_status.v_interlace;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			// 0 = Enabled
			text << "Display Enable: " << (psx->gpu->gpu_status.display_enable == 0);
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Interrupt Request: " << psx->gpu->gpu_status.irq_request;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Ready DMA block: " << psx->gpu->gpu_status.ready_dma;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Ready CMD Word: " << psx->gpu->gpu_status.ready_cmd_word;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Ready VRAM to CPU: " << psx->gpu->gpu_status.ready_vram_to_cpu;
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "DMA Direction: ";
			switch (psx->gpu->gpu_status.dma_direction)
			{
			case 0:
				text << "Off";
				break;
			case 1:
				text << "?";
				break;
			case 2:
				text << "CPUtoGP0";
				break;
			case 3:
				text << "GPUREADtoCPU";
				break;
			}
			ImGui::Text(text.str().c_str());
		}

		{
			std::stringstream text;
			text << "Drawing: " << (psx->gpu->gpu_status.even_odd ? "Odd" : "Even/Vblank");
			ImGui::Text(text.str().c_str());
		}

		// todo add more
	}

	ImGui::End();
}

void DebugMenu::draw_assembly_menu()
{
	ImGui::Begin("Assembly");

	unsigned int pc = psx->cpu->current_pc;

	psx->bus->suppress_exceptions = true;

	for (int idx = -10; idx < 10; idx++)
	{
		std::stringstream asm_text;
		unsigned int pc = psx->cpu->current_pc + static_cast<unsigned int>(idx*4);
		instruction_union instruction = psx->bus->get_word(pc);
		if (pc == psx->cpu->current_pc)
		{
			asm_text << ">>";
		}
		else if (pc == psx->cpu->next_pc)
		{
			asm_text << "->";
		}
		else
		{
			asm_text << "  ";
		}
		asm_text << "0x" << std::hex << std::setfill('0') << std::setw(8) << instruction.raw << "; " << MipsToString::to_string(instruction) << "\n";
		ImGui::Text(asm_text.str().c_str());
		ImGui::SameLine();

		char * buffer = assembly_comment_buffer[pc];
		if (buffer == nullptr)
		{
			buffer = new char[256];
			memset(buffer, 0, 256);
			assembly_comment_buffer[pc] = buffer;
		}

		ImGui::InputText((std::string("##")+std::to_string(idx)).c_str(), buffer, 256);
	}
	psx->bus->suppress_exceptions = false;
	
	ImGui::End();
}

void DebugMenu::draw_controls_menu()
{
	ImGui::Begin("Controls");

	{

		if (ImGui::Button(pause_on_enter_exit_exception ? "Disable pause on enter/exit exception" : "Enable pause on enter/exit exception"))
		{
			pause_on_enter_exit_exception = !pause_on_enter_exit_exception;
		}
	}

	ImGui::End();
}

void DebugMenu::draw_interrupt_menu()
{
	ImGui::Begin("Interrupts");

	{
		std::stringstream text;
		bool enabled = psx->cpu->cop0->interrupt_mask_register.IRQ0_VBLANK;
		bool irq = psx->cpu->cop0->interrupt_status_register.IRQ0_VBLANK;
		text << "IRQ0_VBLANK: " <<  (enabled ? "Enabled " : "Disabled ") << "- " << (irq ? "IRQ" : "No IRQ");
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

	ImGui::End();
}

void DebugMenu::draw_bus_menu()
{
	ImGui::Begin("Bus");

	psx->bus->suppress_exceptions = true;

	static int address_of_interest = 0x0;
	ImGui::InputInt("Address", &address_of_interest, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);

	bool save_enable_pause_state = psx->bus->enable_pause_on_address_access;
	psx->bus->enable_pause_on_address_access = false;
	try
	{
		std::stringstream text;
		text << "Word: 0x" << std::hex << std::setfill('0') << std::setw(8) << psx->bus->get_word(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Word access not supported");
	}

	try
	{
		std::stringstream text;
		text << "Halfword: 0x" << std::hex << std::setfill('0') << std::setw(4) << psx->bus->get_halfword(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Halfword access not supported");
	}

	try
	{
		std::stringstream text;
		text << "Byte: 0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)psx->bus->get_byte(address_of_interest);
		ImGui::Text(text.str().c_str());
	}
	catch (...)
	{
		ImGui::Text("Byte access not supported");
	}

	static int new_value = 0x0;
	ImGui::NewLine();
	ImGui::InputInt("New Value", &new_value, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
	if (ImGui::Button("Apply as Word"))
	{
		psx->bus->set_word(address_of_interest, new_value);
	}

	if (ImGui::Button("Apply as Halfword"))
	{
		psx->bus->set_halfword(address_of_interest, new_value);
	}

	if (ImGui::Button("Apply as Byte"))
	{
		psx->bus->set_byte(address_of_interest, new_value);
	}

	psx->bus->enable_pause_on_address_access = save_enable_pause_state;

	if (ImGui::Button(psx->bus->enable_pause_on_address_access ? "Disable pause on access" : "Enable pause on access"))
	{
		psx->bus->enable_pause_on_address_access = !psx->bus->enable_pause_on_address_access;
		psx->bus->address_to_pause_on = address_of_interest;
	}

	psx->bus->suppress_exceptions = false;

	ImGui::End();
}