#include <memory>
#include <iostream>
#include <fstream>

#include "MemoryMap.hpp"
#include "Dma.hpp"
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "IOPorts.hpp"
#include "glad.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

bool save_state = false;
bool load_state = false;

void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	else if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
	{
		save_state = true;
	}
	else if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
	{
		load_state = true;
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

	std::cout << "Setting up imgui\n";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

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

	std::cout << "Creating IO ports\n";
	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();
	io_ports->init(gpu, dma);

	// RAM
	std::cout << "Creating Ram\n";
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	std::string bios_file(args[1]);
	ram->init(bios_file, io_ports);

	// CPU
	std::cout << "Creating CPU\n";
	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	std::cout << "Hooking up all peripherals to the DMA\n";
	dma->init(ram, gpu);

	std::cout << "Running!\n";

	double current_frame_time = 0.0;
	while (!glfwWindowShouldClose(window))
	{
		auto start_time = glfwGetTime();

		cpu->tick();
		dma->tick();
		gpu->tick();

		if (save_state)
		{
			save_state = false;

			std::cout << "Saving state!\n";

			std::ofstream state_file;
			state_file.open("save_state.bin", std::ios::out | std::ios::binary);

			if (state_file.is_open())
			{
				cpu->save_state(state_file);
				gpu->save_state(state_file);
				dma->save_state(state_file);
				ram->save_state(state_file);
				io_ports->save_state(state_file);

				state_file.close();
				std::cout << "State saved!\n";
			}
			else
			{
				std::cout << "Failed to save state!\n";
			}
		}
		if (load_state)
		{
			load_state = false;
			std::ifstream state_file;
			state_file.open("save_state.bin", std::ios::in | std::ios::binary);

			if (state_file.is_open())
			{
				cpu->load_state(state_file);
				gpu->load_state(state_file);
				dma->load_state(state_file);
				ram->load_state(state_file);
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
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gpu->width, gpu->height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, gpu->video_ram.data());

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
			glfwPollEvents();
			current_frame_time = 0;
		}
	}

	// cleanup
	glDeleteTextures(1, &tex);
	glDeleteProgram(shader_program);
	glDeleteShader(frag_shader);
	glDeleteShader(vert_shader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}