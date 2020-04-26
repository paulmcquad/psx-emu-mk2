#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>

#include "Ram.hpp"
#include "Dma.hpp"
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include "Cdrom.hpp"
#include "IOPorts.hpp"
#include "glad.h"

#include "DebugMenu.hpp"

#include <GLFW/glfw3.h>

constexpr double FRAME_TIME_SECS = 1.0 / 60.0;

static const char* vertex_shader_src =
"#version 150 core\n"
"in vec2 position;\n"
"in vec2 texcoord_in;\n"
"out vec2 texcoord_out;\n"
"void main(){\n"
"texcoord_out = texcoord_in;\n"
"gl_Position = vec4(position, 0.0, 1.0);}\n";

static const char* frag_shader_src =
"#version 150 core\n"
"in vec2 texcoord_out;\n"
"out vec4 color_out;\n"
"uniform sampler2D tex;\n"
"void main(){\n"
"color_out = texture(tex, texcoord_out);}\n";

bool show_debug_menus = true;

void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	else if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
	{
		show_debug_menus = !show_debug_menus;
	}
}

int main(int num_args, char ** args )
{
	if (num_args != 4)
	{
		std::cerr << "Wrong number of arguments, must specify bios path, bin path and cue path\n";
		return -1;
	}

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(FRAME_WIDTH, FRAME_HEIGHT, "PSX-EMU-MK2", nullptr, nullptr);
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
	GLuint vao, vbo, vert_shader, frag_shader, shader_program, tex;
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);

		float vertices[] = {
		//   X   |  Y  |   U  |   V
		   -1.0f,  1.0f,  0.0f,  0.0f,
			1.0f,  1.0f,  1.0f,  0.0f,
		   -1.0f, -1.0f,  0.0f,  1.0f,
		    1.0f,  1.0f,  1.0f,  0.0f,
		    1.0f, -1.0f,  1.0f,  1.0f,
		   -1.0f, -1.0f,  0.0f,  1.0f
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
		glEnableVertexAttribArray(pos_attrib);
		glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

		GLint tex_attrib = glGetAttribLocation(shader_program, "texcoord_in");
		glEnableVertexAttribArray(tex_attrib);
		glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		glGenTextures(1, &tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(glGetUniformLocation(shader_program, "text"), 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Device I/O
	std::shared_ptr<Dma> dma = std::make_shared<Dma>();

	std::cout << "Creating GPU\n";
	std::shared_ptr<Gpu> gpu = std::make_shared<Gpu>();
	gpu->init();

	std::cout << "Creating SPU\n";
	std::shared_ptr<Spu> spu = std::make_shared<Spu>();
	if (spu->init() == false)
	{
		std::cerr << "Failed to initialise SPU\n";
		return -1;
	}

	std::cout << "Creating CDROM\n";
	std::string bin_file(args[2]);
	std::string cue_file(args[3]);

	std::shared_ptr<Cdrom> cdrom = std::make_shared<Cdrom>();
	cdrom->init();
	if (cdrom->load(bin_file, cue_file))
	{
		glfwSetWindowTitle(window, bin_file.c_str());
	}
	else
	{
		std::cerr << "Unable to load ROM\n";
		return -1;
	}

	std::cout << "Creating IO ports\n";
	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();
	io_ports->init(gpu, dma, spu, cdrom);

	// RAM
	std::cout << "Creating Ram\n";
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	std::string bios_file(args[1]);
	if (false == ram->init(bios_file, io_ports))
	{
		return -1;
	}

	// CPU
	std::cout << "Creating CPU\n";
	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	std::cout << "Hooking up all peripherals to the DMA\n";
	dma->init(ram, gpu, spu);

	// setting up debug menu
	std::cout << "Setting up imgui debug menu\n";
	std::shared_ptr<DebugMenu> debug_menu = std::make_shared<DebugMenu>();
	debug_menu->init(window, cpu, gpu, ram, io_ports);

	std::cout << "Running!\n";
	double current_frame_time = 0.0;
	while (!glfwWindowShouldClose(window))
	{
		auto start_time = glfwGetTime();

		if (debug_menu->is_paused() == false || debug_menu->is_step_requested() == true)
		{
			cpu->tick();
			dma->tick();
			gpu->tick();
			cdrom->tick();
			io_ports->tick();

			debug_menu->ticks_per_frame++;
		}

		if (debug_menu->is_save_state_requested())
		{
			std::cout << "Saving state!\n";

			std::ofstream state_file;
			state_file.open("save_state.bin", std::ios::out | std::ios::binary);

			if (state_file.is_open())
			{
				cpu->save_state(state_file);
				gpu->save_state(state_file);
				dma->save_state(state_file);
				ram->save_state(state_file);
				cdrom->save_state(state_file);
				io_ports->save_state(state_file);

				state_file.close();
				std::cout << "State saved!\n";
			}
			else
			{
				std::cout << "Failed to save state!\n";
			}
		}
		if (debug_menu->is_load_state_requested())
		{
			std::ifstream state_file;
			state_file.open("save_state.bin", std::ios::in | std::ios::binary);

			if (state_file.is_open())
			{
				cpu->load_state(state_file);
				gpu->load_state(state_file);
				dma->load_state(state_file);
				ram->load_state(state_file);
				cdrom->load_state(state_file);
				io_ports->load_state(state_file);

				state_file.close();
				std::cout << "State restored!\n";
			}
			else
			{
				std::cout << "Failed to load state!\n";
			}
		}

		auto end_time = glfwGetTime();
		current_frame_time += end_time - start_time;

		if (current_frame_time >= FRAME_TIME_SECS)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gpu->width, gpu->height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, gpu->video_ram);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			if (show_debug_menus)
			{
				debug_menu->draw();
			}

			debug_menu->ticks_per_frame = 0;

			glfwSwapBuffers(window);
			glfwPollEvents();
			current_frame_time = 0;
		}
	}

	debug_menu->uninit();

	// cleanup
	glDeleteTextures(1, &tex);
	glDeleteProgram(shader_program);
	glDeleteShader(frag_shader);
	glDeleteShader(vert_shader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}