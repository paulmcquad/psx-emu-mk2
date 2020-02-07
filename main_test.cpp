#define CATCH_CONFIG_MAIN
#define TESTING

#include <catch.hpp>
#include <string>
#include <memory>

#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "IOPorts.hpp"
#include "Coprocessor0.hpp"

TEST_CASE("Cpu")
{
	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();

	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	ram->init("", io_ports);

	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	SECTION("r0 is always zero")
	{
		REQUIRE(cpu->get_register(0) == 0x0);
		cpu->set_register(0, 0xABCD);
		REQUIRE(cpu->get_register(0) == 0x0);
	}

	SECTION("ensure r1 - r31 work")
	{
		for (int index = 1; index < 32; index++)
		{
			REQUIRE(cpu->get_register(index) == 0x0);
			unsigned int value = 1 << (index - 1);
			cpu->set_register(index, value);
			REQUIRE(cpu->get_register(index) == value);
		}
	}
}