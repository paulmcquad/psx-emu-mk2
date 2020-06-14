cmake_minimum_required(VERSION 3.1)
project (psx-emu-mk2)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw/)
add_subdirectory(glm/)

include_directories(imgui/)
include_directories(glad/)
include_directories(utils/)

include_directories(catch/)

set(imgui_files
		imgui/khrplatform.h
		imgui/imconfig.h
		imgui/imgui_demo.cpp
		imgui/imgui_draw.cpp
		imgui/imgui_impl_opengl3.cpp
		imgui/imgui_impl_opengl3.h
		imgui/imgui_impl_glfw.cpp
		imgui/imgui_impl_glfw.h
		imgui/imgui_internal.h
		imgui/imgui_widgets.cpp
		imgui/imgui.cpp
		imgui/imgui.h
		imgui/imstb_rectpack.h
		imgui/imstb_textedit.h
		imgui/imstb_truetype.h
		imgui/implot.h
		imgui/implot.cpp
)

set(glad_files
	glad/glad.h
	glad/glad.c
)

set(debug_files
		DebugMenuManager.hpp
		DebugMenuManager.cpp
		AssemblyMenu.hpp
		AssemblyMenu.cpp
		MemoryMenu.hpp
		MemoryMenu.cpp
		CpuMenu.hpp
		CpuMenu.cpp
		GpuMenu.hpp
		GpuMenu.cpp
		CdromMenu.hpp
		CdromMenu.cpp
		InterruptMenu.hpp
		InterruptMenu.cpp
		apg_console.h
		apg_console.c
		DebugConsole.hpp
		DebugConsole.cpp
)
	

set(source_files
		Psx.hpp
		Psx.cpp
		Dma.hpp
		Dma.cpp
		Cpu.hpp
		Cpu.cpp
		Ram.hpp
		Ram.cpp
		Rom.hpp
		Rom.cpp
		MemoryControl.hpp
		MemoryControl.cpp
		CacheControl.hpp
		CacheControl.cpp
		ParallelPort.hpp
		ParallelPort.cpp
		Timers.hpp
		Timers.cpp
		RegisterFile.hpp
		RegisterFile.cpp
		Coprocessor.hpp
		SystemControlTypes.hpp
		SystemControlCoprocessor.hpp
		SystemControlCoprocessor.cpp
		GTECoprocessor.hpp
		GTECoprocessor.cpp
		InstructionTypes.hpp
		InstructionEnums.hpp
		Gpu.hpp
		Gpu.cpp
		Spu.hpp
		Spu.cpp
		Cdrom.hpp
		Cdrom.cpp
		Bus.hpp
		Bus.cpp
)

set (test_files
	tests/main_test.cpp
	tests/util_test.cpp
	tests/cpu_test.cpp
	tests/cdrom_test.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${source_files} ${imgui_files} ${glad_files} ${debug_files})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)

add_executable(${PROJECT_NAME}-test ${test_files} ${source_files})
target_link_libraries(${PROJECT_NAME}-test glfw)
target_link_libraries(${PROJECT_NAME}-test glm)