#include "Psx.hpp"
#include "Dma.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include "Cdrom.hpp"
#include "Cpu.hpp"
#include "SystemControlCoprocessor.hpp"
#include "GTECoprocessor.hpp"
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
	Dma * dma = Dma::get_instance();

	Gpu * gpu = Gpu::get_instance();
	gpu->init();
	
	Spu * spu = Spu::get_instance();
	if (spu->init() == false)
	{
		std::cerr << "Failed to initialise SPU\n";
		return false;
	}

	Cdrom * cdrom = Cdrom::get_instance();
	cdrom->init();

	Ram * ram = Ram::get_instance();
	Rom * rom = Rom::get_instance();
	if (false == rom->load_bios(bios_path))
	{
		std::cerr << "Failed to load bios\n";
		return false;
	}

	MemoryControl * memory_control = MemoryControl::get_instance();
	CacheControl * cache_control = CacheControl::get_instance();
	ParallelPort * parallel_port = ParallelPort::get_instance();
	Timers * timers = Timers::get_instance();

	// hook up the bus
	Bus * bus = Bus::get_instance();
	bus->register_device(cdrom);
	bus->register_device(ram);
	bus->register_device(rom);
	bus->register_device(memory_control);
	bus->register_device(cache_control);
	bus->register_device(spu);
	bus->register_device(parallel_port);
	bus->register_device(timers);
	bus->register_device(dma);
	bus->register_device(gpu);

	Cpu * cpu = Cpu::get_instance();
	cpu->init();

	SystemControlCoprocessor * cop0 = SystemControlCoprocessor::get_instance();
	cop0->register_system_control_device(cdrom);
	GTECoprocessor::get_instance();

	bus->register_device(cop0);

	dma->init();
	cop0->register_system_control_device(dma);

	return true;
}


void Psx::tick()
{
	Cpu::get_instance()->tick();
	Dma::get_instance()->tick();
	Gpu::get_instance()->tick();
	Cdrom::get_instance()->tick();
}

void Psx::reset()
{
	Cpu::get_instance()->reset();
	Gpu::get_instance()->reset();
	Ram::get_instance()->reset();
	Cdrom::get_instance()->reset();
	Spu::get_instance()->reset();
	Dma::get_instance()->reset();
}

void Psx::save_state(std::stringstream& state_stream)
{
	Cpu::get_instance()->save_state(state_stream);
	Gpu::get_instance()->save_state(state_stream);
	Dma::get_instance()->save_state(state_stream);
	Ram::get_instance()->save_state(state_stream);
	Cdrom::get_instance()->save_state(state_stream);
}

void Psx::load_state(std::stringstream& state_stream)
{
	Cpu::get_instance()->load_state(state_stream);
	Gpu::get_instance()->load_state(state_stream);
	Dma::get_instance()->load_state(state_stream);
	Ram::get_instance()->load_state(state_stream);
	Cdrom::get_instance()->load_state(state_stream);
}

bool Psx::load(std::string bin_path, std::string cue_path)
{
	return Cdrom::get_instance()->load(bin_path, cue_path);
}