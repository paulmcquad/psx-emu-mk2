#pragma once
#include <memory>
#include <map>
#include <deque>
#include <sstream>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct GLFWwindow;

// must be inherited by any menus you want to add
class DebugMenu
{
public:
	enum class menubar_category { FILE, VIEW, OPTIONS };

	// let's you add different entries into the main menu to control this
	virtual void draw_in_category(menubar_category category) = 0;
	// draw the full menu
	virtual void draw_menu() = 0;
	// called every frame - in case the menu needs to do some logic
	virtual void tick() = 0;
};

class DebugMenuManager
{
public:
	void init(GLFWwindow* window);
	void uninit();
	void draw();
	void tick();

	bool is_paused() { return paused_requested; };

	bool is_forward_step_requested() { 
		bool result = step_forward_requested;
		step_forward_requested = false;
		return result;
	};

	bool is_save_state_requested() {
		bool result = save_state_requested;
		save_state_requested = false;
		return result;
	};

	bool is_load_state_requested() {
		bool result = load_state_requested;
		load_state_requested = false;
		return result;
	};

	bool paused_requested = true;
	bool recording_states = false;
	bool ignore_vram_when_recording = false;

	int max_saved_states = 100;

private:
	std::vector <std::shared_ptr<DebugMenu>> menus;

	void draw_main_menu();

	bool step_forward_requested = false;

	bool save_state_requested = false;
	bool load_state_requested = false;

	std::deque<std::stringstream *> backward_states;
};