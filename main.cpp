#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "IOPorts.hpp"
#include <memory>
#include <iostream>
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int num_args, char ** args )
{
	if (num_args != 2)
	{
		std::cerr << "Wrong number of arguments\n";
		return -1;
	}

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(640, 480, "PSX-EMU-MK2", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);

	// Device I/O
	std::shared_ptr<Gpu> gpu = std::make_shared<Gpu>();

	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();
	io_ports->init(gpu);

	// RAM
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	std::string bios_file(args[1]);
	ram->init(bios_file, io_ports);

	// CPU
	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	while (!glfwWindowShouldClose(window))
	{
		cpu->tick();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// cleanup
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}