#define CATCH_CONFIG_MAIN
#define TESTING

#include <catch.hpp>
#include <string>
#include <memory>

#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "IOPorts.hpp"
#include "Coprocessor0.hpp"
#include "InstructionEnums.hpp"
#include "Exceptions.hpp"

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

	SECTION("ALU")
	{
		/*
		void set_on_less_than_unsigned_immediate(const instruction_union& instr);
		void and_immediate(const instruction_union& instr);
		void or_immediate(const instruction_union& instr);
		void xor_immediate(const instruction_union& instr);
		void load_upper_immediate(const instruction_union& instr);
		*/

		// add immediate [rt] = [rs] + immediate
		{
			// simple addition
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFE);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDI);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = 0x1;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFF);
			}

			// simple addition of negative immediate
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFF);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDI);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = static_cast<short>(-0x1);
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFE);
			}

			// overflow exception thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDI);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = 1;

				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);
			}

			// overflow exception thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDI);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = -1;

				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);
			}
		}

		// add immediate unsigned [rt] = [rs] + immediate
		{
			// simple addition
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFE);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDIU);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = 0x1;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFF);
			}

			// simple addition of negative immediate
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFF);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDIU);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = static_cast<short>(-0x1);
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFE);
			}

			// overflow exception not thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDIU);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = 1;

				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}

			// overflow exception not thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDIU);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = -1;

				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}
		}

		// set on less than immediate [rt] = 1 if [rs] < immediate else 0
		{
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0x10);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::SLTI);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = 0x11;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 1);

				instr.immediate_instruction.immediate = 0x09;
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0);
			}

			// extremes
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);

				instruction_union instr;
				instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::SLTI);
				instr.immediate_instruction.rt = 1;
				instr.immediate_instruction.rs = 2;
				instr.immediate_instruction.immediate = 0;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0);

				cpu->set_register(2, INT_MIN);
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 1);

				cpu->set_register(2, 0x0);
				instr.immediate_instruction.immediate = SHRT_MIN;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0);

				instr.immediate_instruction.immediate = SHRT_MAX;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 1);
			}
		}
	}
}