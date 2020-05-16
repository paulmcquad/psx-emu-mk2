#include "Psx.hpp"
#include "Dma.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include "Cdrom.hpp"
#include "IOPorts.hpp"
#include "Ram.hpp"
#include "Cpu.hpp"

#include <iostream>
#include <fstream>

bool Psx::init(std::string bios_path)
{
	dma = std::make_shared<Dma>();

	gpu = std::make_shared<Gpu>();
	gpu->init();
	
	spu = std::make_shared<Spu>();
	if (spu->init() == false)
	{
		std::cerr << "Failed to initialise SPU\n";
		return false;
	}

	cdrom = std::make_shared<Cdrom>();
	cdrom->init();

	io_ports = std::make_shared<IOPorts>();
	io_ports->init(gpu, dma, spu, cdrom);

	ram = std::make_shared<Ram>();
	ram->init(io_ports);
	if (false == ram->load_bios(bios_path))
	{
		std::cerr << "Failed to load bios\n";
		return false;
	}

	cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	dma->init(ram, gpu, spu);

	return true;
}


void Psx::tick()
{
	cpu->tick();
	dma->tick();
	gpu->tick();
	cdrom->tick();
	io_ports->tick();
}

void Psx::save_state(std::string state_file_path)
{
	std::cout << "Saving state!\n";

	std::ofstream state_file;
	state_file.open(state_file_path, std::ios::out | std::ios::binary);

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

void Psx::load_state(std::string state_file_path)
{
	std::ifstream state_file;
	state_file.open(state_file_path, std::ios::in | std::ios::binary);

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

bool Psx::load(std::string bin_path, std::string cue_path)
{
	return cdrom->load(bin_path, cue_path);
}