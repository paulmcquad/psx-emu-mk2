#pragma once

#include <memory>
#include <map>
#include <deque>
#include <sstream>

struct GLFWwindow;
class Psx;

class DebugMenu
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

	bool show_assembly_window = false;
	bool show_memory_window = false;
	bool show_cpu_window = false;
	bool show_gpu_window = false;
	bool show_interrupt_window = false;
	bool show_cdrom_window = false;

	void draw_main_menu();
	void draw_cpu_menu();
	void draw_gpu_menu();
	void draw_assembly_menu();
	void draw_interrupt_menu();
	void draw_bus_menu();
	void draw_cdrom_menu();

	std::shared_ptr<Psx> psx = nullptr;

	bool step_forward_requested = false;

	bool save_state_requested = false;
	bool load_state_requested = false;

	std::map<unsigned int, char *> assembly_comment_buffer;
	std::deque<std::stringstream *> backward_states;
};