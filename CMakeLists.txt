cmake_minimum_required(VERSION 3.1)
project (psx-emu-mk2)

include_directories(includes/)

set(source_files
		Cpu.hpp
		Cpu.cpp
		MemoryMap.hpp
		MemoryMap.cpp
		Coprocessor.hpp
		Coprocessor0.hpp
		Coprocessor0.cpp
		Coprocessor2.hpp
		Coprocessor2.cpp
		InstructionTypes.hpp
		InstructionEnums.hpp
		IOPorts.hpp
		IOPorts.cpp
		Gpu.hpp
		Gpu.cpp
		Exceptions.hpp
)

add_executable(psx-mk2
			main.cpp
            ${source_files})

add_executable(psx-mk2-test 
            main_test.cpp
			${source_files})