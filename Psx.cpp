#include "Psx.hpp"
#include "Dma.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include "Cdrom.hpp"
#include "Cpu.hpp"
#include "SystemControlCoprocessor.hpp"
#include "Bus.hpp"
#include "Ram.hpp"
#include "Rom.hpp"
#include "MemoryControl.hpp"
#include "CacheControl.hpp"
#include "ParallelPort.hpp"
#include "Timers.hpp"

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

	ram = std::make_shared<Ram>();
	
	rom = std::make_shared<Rom>();
	if (false == rom->load_bios(bios_path))
	{
		std::cerr << "Failed to load bios\n";
		return false;
	}

	memory_control = std::make_shared<MemoryControl>();

	cache_control = std::make_shared<CacheControl>();

	parallel_port = std::make_shared<ParallelPort>();

	timers = std::make_shared<Timers>();

	// hook up the bus
	bus = std::make_shared<Bus>();
	bus->register_device(cdrom.get());
	bus->register_device(ram.get());
	bus->register_device(rom.get());
	bus->register_device(memory_control.get());
	bus->register_device(cache_control.get());
	bus->register_device(spu.get());
	bus->register_device(parallel_port.get());
	bus->register_device(timers.get());
	bus->register_device(dma.get());
	bus->register_device(gpu.get());

	cpu = std::make_shared<Cpu>();
	cpu->init(bus);

	cpu->cop0->register_system_control_device(cdrom.get());

	bus->register_device(cpu->cop0.get());

	dma->init(bus, gpu, spu);

	return true;
}


void Psx::tick()
{
	cpu->tick();
	dma->tick();
	gpu->tick();
	cdrom->tick();
}

void Psx::save_state(std::stringstream& state_stream)
{
	cpu->save_state(state_stream);
	gpu->save_state(state_stream);
	dma->save_state(state_stream);
	ram->save_state(state_stream);
	cdrom->save_state(state_stream);
}

void Psx::load_state(std::stringstream& state_stream)
{
	cpu->load_state(state_stream);
	gpu->load_state(state_stream);
	dma->load_state(state_stream);
	ram->load_state(state_stream);
	cdrom->load_state(state_stream);
}

bool Psx::load(std::string bin_path, std::string cue_path)
{
	return cdrom->load(bin_path, cue_path);
}