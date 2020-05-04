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
include_directories(SDL2/include/)
link_directories(SDL2/lib)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    link_directories(SDL2/lib/x64/)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    link_directories(SDL2/lib/x86/)
endif()



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
)

set(glad_files
	glad/glad.h
	glad/glad.c
)

set(source_files
		Dma.hpp
		Dma.cpp
		Cpu.hpp
		Cpu.cpp
		Ram.hpp
		Ram.cpp
		RegisterFile.hpp
		RegisterFile.cpp
		Coprocessor.hpp
		SystemControlCoprocessor.hpp
		SystemControlCoprocessor.cpp
		GTECoprocessor.hpp
		GTECoprocessor.cpp
		InstructionTypes.hpp
		InstructionEnums.hpp
		IOPorts.hpp
		IOPorts.cpp
		Gpu.hpp
		Gpu.cpp
		Spu.hpp
		Spu.cpp
		Cdrom.hpp
		Cdrom.cpp
		Exceptions.hpp
)

add_executable(${PROJECT_NAME} main.cpp ${source_files} ${imgui_files} ${glad_files} DebugMenu.hpp DebugMenu.cpp)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} SDL2)

add_executable(${PROJECT_NAME}-test main_test.cpp ${source_files})
target_link_libraries(${PROJECT_NAME}-test glfw)
target_link_libraries(${PROJECT_NAME}-test glm)
target_link_libraries(${PROJECT_NAME}-test SDL2)