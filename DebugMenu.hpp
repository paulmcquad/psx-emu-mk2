#pragma once

#include <memory>

struct GLFWwindow;
class Cpu;
class Gpu;

class DebugMenu
{
public:
	void init(GLFWwindow* window, std::shared_ptr<Cpu> _cpu, std::shared_ptr<Gpu> _gpu);
	void deinit();
	void draw();

	bool is_paused() { return paused_requested; };

	bool is_step_requested() { 
		bool result = step_requested;
		step_requested = false;
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

private:

	void draw_cpu_menu();
	void draw_gpu_menu();
	
	void draw_controls_menu();
	void draw_overview_menu();

	std::shared_ptr<Cpu> cpu = nullptr;
	std::shared_ptr<Gpu> gpu = nullptr;

	bool paused_requested = false;
	bool step_requested = false;
	bool save_state_requested = false;
	bool load_state_requested = false;
};