#define CATCH_CONFIG_MAIN
#define TESTING

#include <catch.hpp>
#include <string>
#include <memory>
#include <limits>

#include "Ram.hpp"
#include "Cpu.hpp"
#include "IOPorts.hpp"
#include "SystemControlCoprocessor.hpp"
#include "InstructionEnums.hpp"
#include "Exceptions.hpp"

TEST_CASE("Standard Opcodes")
{
	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();

	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	ram->init("", io_ports);

	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	// make sure the cache isn't isolated
	SystemControlCoprocessor::status_register status = cpu->cop0->get<SystemControlCoprocessor::status_register>();
	status.Isc = false;
	cpu->cop0->set<SystemControlCoprocessor::status_register>(status);

	// Add Immediate Word
	// add rt, rs, imm
	// rt = rs + (signed) imm
	SECTION("ADDI")
	{
		cpu->register_file.reset();

		// add positive
		{
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, 10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 10);
		}

		// add negative
		{
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, -10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 0);
		}

		// test for overflow
		{
			cpu->register_file.set_register(1, std::numeric_limits<int>::max());
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::max());
			REQUIRE_THROWS_AS(cpu->execute(instruction), overflow_exception);
		}

		{
			cpu->register_file.set_register(1, std::numeric_limits<int>::min());
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::min());
			REQUIRE_THROWS_AS(cpu->execute(instruction), overflow_exception);
		}

		{
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::max());
			REQUIRE_NOTHROW(cpu->execute(instruction));
		}

		{
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::min());
			REQUIRE_NOTHROW(cpu->execute(instruction));
		}
	}

	SECTION("ADDIU")
	{

	}

	SECTION("SLTI")
	{

	}

	SECTION("SLTIU")
	{

	}

	SECTION("ANDI")
	{

	}

	SECTION("ORI")
	{

	}

	SECTION("XORI")
	{

	}

	SECTION("LUI")
	{

	}
}

TEST_CASE("Normal branching opcodes")
{
	SECTION("J")
	{

	}

	SECTION("JAL")
	{

	}

	SECTION("BEQ")
	{

	}

	SECTION("BNE")
	{

	}

	SECTION("BLEZ")
	{

	}

	SECTION("BGTZ")
	{

	}
}

TEST_CASE("Special Opcodes")
{
	SECTION("SLL")
	{

	}

	SECTION("SRL")
	{

	}

	SECTION("SRA")
	{

	}

	SECTION("SLLV")
	{

	}

	SECTION("SRLV")
	{

	}

	SECTION("SRAV")
	{

	}

	SECTION("JR")
	{

	}

	SECTION("JALR")
	{

	}

	SECTION("SYSCALL")
	{

	}

	SECTION("BREAK")
	{

	}

	SECTION("MFHI")
	{

	}

	SECTION("MTHI")
	{

	}

	SECTION("MFLO")
	{

	}

	SECTION("MTLO")
	{

	}

	SECTION("MULT")
	{

	}

	SECTION("MULTU")
	{

	}

	SECTION("DIV")
	{

	}

	SECTION("DIVU")
	{

	}

	SECTION("ADD")
	{

	}

	SECTION("ADDU")
	{

	}

	SECTION("ADDU")
	{

	}

	SECTION("SUB")
	{

	}

	SECTION("SUBU")
	{

	}

	SECTION("AND")
	{

	}

	SECTION("OR")
	{

	}

	SECTION("XOR")
	{

	}

	SECTION("NOR")
	{

	}

	SECTION("SLT")
	{

	}

	SECTION("SLTU")
	{

	}
}

TEST_CASE("Bcond opcodes")
{
	SECTION("BLTZ")
	{

	}

	SECTION("BGEZ")
	{

	}

	SECTION("BLTZAL")
	{

	}

	SECTION("BGEZAL")
	{

	}
}

TEST_CASE("Memory access opcodes")
{
	SECTION("LB")
	{

	}

	SECTION("LH")
	{

	}

	SECTION("LWL")
	{

	}

	SECTION("LW")
	{

	}

	SECTION("LBU")
	{

	}

	SECTION("LHU")
	{

	}

	SECTION("LWR")
	{

	}

	SECTION("SB")
	{

	}

	SECTION("SH")
	{

	}

	SECTION("SWL")
	{

	}

	SECTION("SW")
	{

	}

	SECTION("SWR")
	{

	}
}