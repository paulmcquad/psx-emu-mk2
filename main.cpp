#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>

#include "Psx.hpp"

#include "Ram.hpp"
#include "Dma.hpp"
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include "Cdrom.hpp"
#include "glad.h"

#include "DebugMenuManager.hpp"

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

	std::cout << "Create PSX\n";
	std::string bios_file(args[1]);
	std::string bin_file(args[2]);
	std::string cue_file(args[3]);

	std::shared_ptr<Psx> psx = std::make_unique<Psx>();
	if (psx->init(bios_file) == false)
	{
		std::cerr << "Unable to initialise PSX\n";
		return -1;
	}

	if (psx->load(bin_file, cue_file) == false)
	{
		std::cerr << "Unable to load game\n";
		return -1;
	}

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(psx->gpu->FRAME_WIDTH, psx->gpu->FRAME_HEIGHT, "PSX-EMU-MK2", nullptr, nullptr);
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


	// setting up debug menu
	std::cout << "Setting up imgui debug menu\n";
	std::shared_ptr<DebugMenuManager> debug_menu = std::make_shared<DebugMenuManager>();
	debug_menu->init(window, psx);

	std::cout << "Running!\n";
	double current_frame_time = 0.0;
	int ticks_per_frame = 0;
	while (!glfwWindowShouldClose(window))
	{
		auto start_time = glfwGetTime();


		if (debug_menu->is_paused() == false || debug_menu->is_forward_step_requested() == true)
		{
			psx->tick();
			debug_menu->tick();

			// allow debug menu to pause on address access
			bool exception_pause_request = (debug_menu->pause_on_enter_exit_exception && psx->cpu->currently_entering_exiting_exeception);
			bool device_pause_request = (debug_menu->pause_on_access_perhipheral && psx->bus->currently_accessing_peripheral);
			
			bool register_pause_request = false;
			if (psx->cpu->register_file.register_just_changed)
			{
				int idx = psx->cpu->register_file.index_of_register_changed;
				if (psx->cpu->register_file.break_on_change[idx])
				{
					if (psx->cpu->register_file.only_pause_on_value)
					{
						if (psx->cpu->register_file.value_of_register_changed == (unsigned int)psx->cpu->register_file.pause_on_value)
						{
							register_pause_request = true;
							psx->cpu->register_file.register_just_changed = false;
						}
					}
					else
					{
						register_pause_request = true;
						psx->cpu->register_file.register_just_changed = false;
					}
				}
			}

			// cancel pause request if the device is being ignored,
			// we don't want it stopping on every device access, it would take forever to get anywhere
			if (device_pause_request)
			{
				switch (psx->bus->device_being_accessed)
				{
					case Bus::BusDevice::bus_device_type::GPU:
					{
						if (debug_menu->ignore_pause_on_access_gpu)
						{
							device_pause_request = false;
						}
					} break;

					case Bus::BusDevice::bus_device_type::SPU:
					{
						if (debug_menu->ignore_pause_on_access_spu)
						{
							device_pause_request = false;
						}
					} break;

					case Bus::BusDevice::bus_device_type::CDROM:
					{
						if (debug_menu->ignore_pause_on_access_cdrom)
						{
							device_pause_request = false;
						}
					} break;

					case Bus::BusDevice::bus_device_type::INTERRUPT_CONTROL:
					{
						if (debug_menu->ignore_pause_on_interrupt_control)
						{
							device_pause_request = false;
						}
					} break;

					default:
					{
						// do nothing
					}
				}
			}
			psx->bus->currently_accessing_peripheral = false;

			bool overwrite_request_pause = psx->cpu->register_file.break_on_overwrite && psx->cpu->register_file.register_just_overwritten;
			psx->cpu->register_file.register_just_overwritten = false;

			if (psx->bus->request_pause || exception_pause_request || device_pause_request || register_pause_request || overwrite_request_pause)
			{
				debug_menu->paused_requested = true;
				psx->bus->request_pause = false;
			}

			ticks_per_frame++;
		}

		if (debug_menu->is_save_state_requested())
		{
			std::cout << "Saving state!\n";

			std::ofstream state_file;
			state_file.open("save_state.bin", std::ios::out | std::ios::binary);

			if (state_file.is_open())
			{
				std::stringstream state_stream;
				psx->save_state(state_stream);
				state_file << state_stream.rdbuf();
				state_file.close();
			}
			
		}
		if (debug_menu->is_load_state_requested())
		{
			std::ifstream state_file;
			state_file.open("save_state.bin", std::ios::in | std::ios::binary);
			if (state_file.is_open())
			{
				std::stringstream state_stream;
				state_stream << state_file.rdbuf();
				psx->load_state(state_stream);
				state_file.close();
			}
		}

		auto end_time = glfwGetTime();
		current_frame_time += end_time - start_time;

		if (current_frame_time >= FRAME_TIME_SECS)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, psx->gpu->width, psx->gpu->height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, psx->gpu->video_ram);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			if (show_debug_menus)
			{
				debug_menu->draw();
			}

			std::stringstream title_text;
			title_text << "PSX-EMU-MK2 FPS: " << std::dec<< 1.0f / current_frame_time << " Ticks Per Frame: " << ticks_per_frame;
			glfwSetWindowTitle(window, title_text.str().c_str());
			ticks_per_frame = 0;

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