#include "DebugMenuManager.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Psx.hpp"
#include "AssemblyMenu.hpp"
#include "MemoryMenu.hpp"
#include "CpuMenu.hpp"
#include "GpuMenu.hpp"
#include "CdromMenu.hpp"

#include <iostream>
#include <sstream>

void DebugMenuManager::init(GLFWwindow* window, std::shared_ptr<Psx> _psx)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

	psx = _psx;

	menus.push_back(std::make_shared<AssemblyMenu>(psx));
	menus.push_back(std::make_shared<MemoryMenu>(psx));
	menus.push_back(std::make_shared<CpuMenu>(psx));
	menus.push_back(std::make_shared<GpuMenu>(psx));
	menus.push_back(std::make_shared<CdromMenu>(psx));
}

void DebugMenuManager::uninit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	for (auto iter : backward_states)
	{
		delete iter;
	}
	backward_states.clear();
}

void DebugMenuManager::draw()
{
	// draw imgui debug menu
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	draw_main_menu();

	for (auto& iter : menus)
	{
		iter->draw_menu();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugMenuManager::tick()
{
	if (recording_states)
	{
		if (backward_states.size() > max_saved_states)
		{
			delete backward_states.front();
			backward_states.pop_front();
		}

		std::stringstream * state = new std::stringstream();
		psx->save_state(*state);
		backward_states.push_back(state);
	}
}

void DebugMenuManager::draw_main_menu()
{
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Reset")) { psx->reset(); }
		if (ImGui::MenuItem("Save state")) { save_state_requested = true; }
		if (ImGui::MenuItem("Load state")) { load_state_requested = true; }

		for (auto& iter : menus)
		{
			iter->draw_in_category(DebugMenu::menubar_category::FILE);
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View"))
	{
		for (auto& iter : menus)
		{
			iter->draw_in_category(DebugMenu::menubar_category::VIEW);
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Options"))
	{
		ImGui::InputInt("Number of backward steps to record", &max_saved_states);

		if (max_saved_states < 0)
		{
			max_saved_states = 0;
		}

		ImGui::Checkbox("Pause on enter/exit interrupt", &pause_on_enter_exit_exception);
		ImGui::Checkbox("Pause on access peripheral", &pause_on_access_perhipheral);
		ImGui::Checkbox("Ignore GPU Access", &ignore_pause_on_access_gpu);
		ImGui::Checkbox("Ignore SPU Access", &ignore_pause_on_access_spu);
		ImGui::Checkbox("Ignore CDROM Access", &ignore_pause_on_access_cdrom);
		ImGui::Checkbox("Ignore Interrupt Control Access", &ignore_pause_on_interrupt_control);

		for (auto& iter : menus)
		{
			iter->draw_in_category(DebugMenu::menubar_category::OPTIONS);
		}

		ImGui::EndMenu();
	}

	ImGui::Separator();
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
	if (ImGui::MenuItem(recording_states ? "[]" : "()")) { 
		recording_states = !recording_states;
		if (recording_states == false)
		{
			backward_states.clear();
		}
	}

	// step forwards
	if (ImGui::MenuItem("->")) { step_forward_requested = true; }
	ImGui::Separator();

	ImGui::EndMainMenuBar();
}