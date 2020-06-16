#include "DebugMenuManager.hpp"

#include "Psx.hpp"
#include "AssemblyMenu.hpp"
#include "MemoryMenu.hpp"
#include "CpuMenu.hpp"
#include "GpuMenu.hpp"
#include "CdromMenu.hpp"
#include "InterruptMenu.hpp"
#include "DebugConsole.hpp"

#include <iostream>
#include <sstream>

void DebugMenuManager::init(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

	menus.push_back(std::make_shared<AssemblyMenu>());
	menus.push_back(std::make_shared<MemoryMenu>());
	menus.push_back(std::make_shared<CpuMenu>());
	menus.push_back(std::make_shared<GpuMenu>());
	menus.push_back(std::make_shared<CdromMenu>());
	menus.push_back(std::make_shared<InterruptMenu>());
	menus.push_back(std::make_shared<DebugConsole>());
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
		if (backward_states.size() > static_cast<unsigned int>(max_saved_states))
		{
			delete backward_states.front();
			backward_states.pop_front();
		}

		std::stringstream * state = new std::stringstream();
		Psx::get_instance()->save_state(*state);
		backward_states.push_back(state);
	}

	for (auto& iter : menus)
	{
		iter->tick();
	}
}

void DebugMenuManager::draw_main_menu()
{
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Reset")) { Psx::get_instance()->reset(); }
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
		Psx::get_instance()->load_state(*state);
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