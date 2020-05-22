#pragma once

#include <memory>

struct GLFWwindow;
class Cpu;
class Gpu;
class Ram;
class IOPorts;

class DebugMenu
{
public:
	void init(GLFWwindow* window, std::shared_ptr<Cpu> _cpu, std::shared_ptr<Gpu> _gpu);
	void uninit();
	void draw();

	bool is_paused() { return paused_requested; };

	bool is_forward_step_requested() { 
		bool result = step_forward_requested;
		step_forward_requested = false;
		return result;
	};

	bool is_backward_step_requested() {
		bool result = step_backward_requested;
		step_backward_requested = false;
		return result;
	}

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

	long ticks_per_frame = 0;

private:

	void draw_cpu_menu();
	void draw_gpu_menu();
	
	void draw_controls_menu();
	void draw_interrupt_menu();

	std::shared_ptr<Cpu> cpu = nullptr;
	std::shared_ptr<Gpu> gpu = nullptr;
	std::shared_ptr<IOPorts> io_ports = nullptr;

	bool paused_requested = true;
	bool step_forward_requested = false;
	bool step_backward_requested = false;

	bool save_state_requested = false;
	bool load_state_requested = false;
};