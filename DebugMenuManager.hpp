#pragma once

#include <memory>
#include <map>
#include <deque>
#include <sstream>
#include <vector>

struct GLFWwindow;
class Psx;

// must be inherited by any menus you want to add
class DebugMenu
{
public:
	enum class menubar_category { FILE, VIEW, OPTIONS };

	DebugMenu(std::shared_ptr<Psx> _psx) { psx = _psx; };

	// let's you add different entries into the main menu to control this
	virtual void draw_in_category(menubar_category category) = 0;
	// draw the full menu
	virtual void draw_menu() = 0;
	// called every frame - in case the menu needs to do some logic
	virtual void tick() = 0;

	std::shared_ptr<Psx> psx = nullptr;
};

class DebugMenuManager
{
public:
	void init(GLFWwindow* window, std::shared_ptr<Psx> _psx);
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
	bool pause_on_enter_exit_exception = false;

	static const int MAX_BACKWARDS_STATES_SAVED = 100;

private:
	std::vector <std::shared_ptr<DebugMenu>> menus;

	bool show_gpu_window = false;
	bool show_interrupt_window = false;
	bool show_cdrom_window = false;

	void draw_main_menu();
	void draw_gpu_menu();
	void draw_interrupt_menu();
	void draw_cdrom_menu();

	std::shared_ptr<Psx> psx = nullptr;

	bool step_forward_requested = false;

	bool save_state_requested = false;
	bool load_state_requested = false;

	std::deque<std::stringstream *> backward_states;
};