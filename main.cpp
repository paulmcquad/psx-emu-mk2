#include "Ram.hpp"
#include "Cpu.hpp"
#include <memory>

int main(int num_args, char * args )
{
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>(ram);



	return 0;
}