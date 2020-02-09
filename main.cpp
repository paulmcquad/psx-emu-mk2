#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "IOPorts.hpp"
#include <memory>
#include <iostream>
#include "glad.h"
#include <GLFW/glfw3.h>

constexpr double FRAME_TIME_SECS = 1.0 / 60.0;

static const char* vertex_shader_src =
"#version 150 core\n"
"in vec2 position;\n"
"void main(){\n"
"gl_Position = vec4(position, 0.0, 1.0);}\n";

static const char* frag_shader_src =
"#version 150 core\n"
"out vec4 outColor;\n"
"void main(){\n"
"outColor = vec4(1.0, 1.0, 1.0, 1.0);}\n";

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

	std::cout << "Creating context\n";
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);

	std::cout << "Initializing GLAD\n";
	if (!gladLoadGL()) {
		std::cerr << "Failed to initialize GLAD\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	std::cout << "Setting up OpenGL\n";
	GLuint vao, vbo, vert_shader, frag_shader, shader_program;
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);

		float vertices[] = {
			0.0f,  0.5f,
			0.5f, -0.5f,
		   -0.5f, -0.5f
		};

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		vert_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_shader, 1, &vertex_shader_src, nullptr);
		glCompileShader(vert_shader);

		frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_shader, 1, &frag_shader_src, nullptr);
		glCompileShader(frag_shader);

		shader_program = glCreateProgram();
		glAttachShader(shader_program, vert_shader);
		glAttachShader(shader_program, frag_shader);

		glLinkProgram(shader_program);
		glUseProgram(shader_program);

		GLint pos_attrib = glGetAttribLocation(shader_program, "position");
		glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(pos_attrib);
	}

	// Device I/O
	std::cout << "Creating GPU\n";
	std::shared_ptr<Gpu> gpu = std::make_shared<Gpu>();
	gpu->init();

	std::cout << "Creating IO ports\n";
	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();
	io_ports->init(gpu);

	// RAM
	std::cout << "Creating Ram\n";
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	std::string bios_file(args[1]);
	ram->init(bios_file, io_ports);

	// CPU
	std::cout << "Creating CPU\n";
	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	std::cout << "Running!\n";

	double current_frame_time = 0.0;
	while (!glfwWindowShouldClose(window))
	{
		auto start_time = glfwGetTime();

		cpu->tick();
		gpu->tick();

		auto end_time = glfwGetTime();
		current_frame_time += end_time - start_time;

		if (current_frame_time >= FRAME_TIME_SECS)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 3);

			glfwSwapBuffers(window);
			glfwPollEvents();
			current_frame_time = 0;
		}
	}

	// cleanup
	glDeleteProgram(shader_program);
	glDeleteShader(frag_shader);
	glDeleteShader(vert_shader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}