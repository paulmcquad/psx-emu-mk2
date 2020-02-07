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
		cpu->set_register(0, 0xDEADBEEF);
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

	SECTION("little endian load/store")
	{
		REQUIRE(ram->load<unsigned int>(0) == 0x0);
		REQUIRE(ram->load<unsigned short>(0) == 0x0);
		REQUIRE(ram->load<unsigned char>(0) == 0x0);

		ram->store<unsigned int>(0, 0xDEADBEEF);
		REQUIRE(ram->load<unsigned int>(0) == 0xDEADBEEF);

		REQUIRE(ram->load<unsigned char>(0) == 0xEF);
		REQUIRE(ram->load<unsigned char>(1) == 0xBE);
		REQUIRE(ram->load<unsigned char>(2) == 0xAD);
		REQUIRE(ram->load<unsigned char>(3) == 0xDE);

		REQUIRE(ram->load<unsigned short>(0) == 0xBEEF);
		REQUIRE(ram->load<unsigned short>(2) == 0xDEAD);

		ram->store<unsigned short>(0, 0xDEAD);
		REQUIRE(ram->load<unsigned int>(0) == 0xDEADDEAD);

		ram->store<unsigned char>(0, 0xEE);
		REQUIRE(ram->load<unsigned int>(0) == 0xDEADDEEE);
	}
}