#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "IOPorts.hpp"
#include <memory>

int main(int num_args, char ** args )
{
	if (num_args != 2)
	{
		return -1;
	}

	// Device I/O
	std::shared_ptr<Gpu> gpu = std::make_shared<Gpu>();

	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();
	io_ports->init(gpu);

	// RAM
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	std::string bios_file(args[1]);
	ram->init(bios_file, io_ports);

	// CPU
	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	while (true)
	{
		cpu->tick();
	}

	return 0;
}