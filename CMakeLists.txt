cmake_minimum_required(VERSION 3.1)
project (psx-emu-mk2)

include_directories(includes/)

add_executable(psx-mk2 
               main.cpp
			   Cpu.hpp
			   Cpu.cpp
			   Ram.hpp
			   Ram.cpp
			   Coprocessor.hpp
			   InstructionTypes.hpp)

add_executable(psx-mk2-test 
               main_test.cpp)