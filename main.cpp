#include "Ram.hpp"
#include "Cpu.hpp"
#include <memory>

int main(int num_args, char ** args )
{
	if (num_args != 2)
	{
		return -1;
	}

	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	std::string bios_file(args[1]);
	ram->init(bios_file);

	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	while (true)
	{
		cpu->run_cycle();
	}

	return 0;
}