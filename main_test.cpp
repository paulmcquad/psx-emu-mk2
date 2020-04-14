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
	// throws exception on overflow
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

	// Add Immediate Unsigned Word
	// add rt, rs, imm
	// rt = rs + imm
	SECTION("ADDIU")
	{
		cpu->register_file.reset();

		// add positive
		{
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, 10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 10);
		}

		// add negative
		{
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, -10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 0);
		}

		// test for overflow - the same as above just all except no throw
		{
			cpu->register_file.set_register(1, std::numeric_limits<int>::max());
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::max());
			REQUIRE_NOTHROW(cpu->execute(instruction));
		}

		{
			cpu->register_file.set_register(1, std::numeric_limits<int>::min());
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::min());
			REQUIRE_NOTHROW(cpu->execute(instruction));
		}

		{
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::max());
			REQUIRE_NOTHROW(cpu->execute(instruction));
		}

		{
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::min());
			REQUIRE_NOTHROW(cpu->execute(instruction));
		}
	}

	// Set on Less Than Immediate
	// rt = (rs < immediate)
	SECTION("SLTI")
	{
		cpu->register_file.reset();

		// register is negative
		{
			instruction_union instruction(cpu_instructions::SLTI, 1, 1, 0);

			cpu->register_file.set_register(1, -10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 1);
		}

		// register is positive
		{
			instruction_union instruction(cpu_instructions::SLTI, 1, 1, 0);

			cpu->register_file.set_register(1, 10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 0);
		}

		// immediate is positive
		{
			instruction_union instruction(cpu_instructions::SLTI, 1, 1, 10);

			cpu->register_file.set_register(1, 0);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 1);
		}

		// immediate is negative
		{
			instruction_union instruction(cpu_instructions::SLTI, 1, 1, -10);

			cpu->register_file.set_register(1, 0);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 0);
		}
	}

	// Set on Less Than Immediate Unsigned
	// SLTIU rt, rs, imm
	SECTION("SLTIU")
	{
		cpu->register_file.reset();

		// register is negative - which being unsigned is really positive
		{
			instruction_union instruction(cpu_instructions::SLTIU, 1, 1, 0);

			cpu->register_file.set_register(1, -10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 0);
		}

		// register is positive
		{
			instruction_union instruction(cpu_instructions::SLTIU, 1, 1, 0);

			cpu->register_file.set_register(1, 10);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 0);
		}

		// immediate is positive
		{
			instruction_union instruction(cpu_instructions::SLTIU, 1, 1, 10);

			cpu->register_file.set_register(1, 0);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 1);
		}

		// immediate is negative - which will actually be a positive as its unsigned
		{
			instruction_union instruction(cpu_instructions::SLTIU, 1, 1, -10);

			cpu->register_file.set_register(1, 0);
			cpu->execute(instruction);

			REQUIRE(cpu->register_file.get_register(1) == 1);
		}
	}

	// AND immediate
	// ANDI rt, rs, imm
	SECTION("ANDI")
	{
		cpu->register_file.reset();

		instruction_union instruction(cpu_instructions::ANDI, 1, 1, 0xF0F0);

		cpu->register_file.set_register(1, 0xFFFFFFFF);
		cpu->execute(instruction);

		REQUIRE(cpu->register_file.get_register(1) == 0xF0F0);
	}

	// OR immediate
	// ORI rt, rs, imm
	SECTION("ORI")
	{
		instruction_union instruction(cpu_instructions::ORI, 1, 1, 0xF0F0);

		cpu->register_file.set_register(1, 0xFFFF0000);
		cpu->execute(instruction);

		REQUIRE(cpu->register_file.get_register(1) == 0xFFFFF0F0);
	}

	// Exclusive OR immediate
	// XORI rt, rs, immediate
	SECTION("XORI")
	{
		instruction_union instruction(cpu_instructions::XORI, 1, 1, 0xFF0F);

		cpu->register_file.set_register(1, 0xFFFFFFFF);
		cpu->execute(instruction);

		REQUIRE(cpu->register_file.get_register(1) == 0xFFFF00F0);
	}

	// Load Upper Immediate
	// LUI rt, immediate
	SECTION("LUI")
	{
		instruction_union instruction(cpu_instructions::LUI, 1, 1, 0xFFFF);

		cpu->register_file.set_register(1, 0x0000FFFF);
		cpu->execute(instruction);

		// LUI fills in the lower 16 bits with zeros regardless of previous content
		REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
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
	std::shared_ptr<IOPorts> io_ports = std::make_shared<IOPorts>();

	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	ram->init("", io_ports);

	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(ram);

	// make sure the cache isn't isolated
	SystemControlCoprocessor::status_register status = cpu->cop0->get<SystemControlCoprocessor::status_register>();
	status.Isc = false;
	cpu->cop0->set<SystemControlCoprocessor::status_register>(status);

	// Shift Word Left Logical
	// SLL rd, rt, sa
	// rd = rt << sa
	SECTION("SLL")
	{
		instruction_union instruction(cpu_instructions::SPECIAL, 0, 2, 1, 16, cpu_special_funcs::SLL);

		// rt
		cpu->register_file.set_register(2, 0xFFFF);

		// rd
		cpu->register_file.set_register(1, 0x0);

		cpu->execute(instruction);

		// LUI fills in the lower 16 bits with zeros regardless of previous content
		REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
	}

	// Shift Word Right Logical
	// SRL rd, rt, sa
	// rd = rd >> sa
	SECTION("SRL")
	{
		instruction_union instruction(cpu_instructions::SPECIAL, 0, 2, 1, 16, cpu_special_funcs::SRL);
		// rt
		cpu->register_file.set_register(2, 0xFFFF0000);
		// rd
		cpu->register_file.set_register(1, 0x0);

		cpu->execute(instruction);

		// LUI fills in the lower 16 bits with zeros regardless of previous content
		REQUIRE(cpu->register_file.get_register(1) == 0xFFFF);
	}

	// Shift Word Right Arithmetic
	// SRA rd, rt, sa
	SECTION("SRA")
	{
		instruction_union instruction(cpu_instructions::SPECIAL, 0, 2, 1, 16, cpu_special_funcs::SRA);
		// rt
		cpu->register_file.set_register(2, 0xFFFF0000);
		// rd
		cpu->register_file.set_register(1, 0x0);

		cpu->execute(instruction);
		// LUI fills in the lower 16 bits with zeros regardless of previous content
		REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFF);
	}

	// Shift World Left Logical Variable
	// SLLV rd, rt, rs
	// rd = rt << rs
	SECTION("SLLV")
	{
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 3, 2, 1, 0, cpu_special_funcs::SLLV);

			// rs
			cpu->register_file.set_register(3, 16);
			// rt
			cpu->register_file.set_register(2, 0xFFFF);
			// rd
			cpu->register_file.set_register(1, 0x0);

			cpu->execute(instruction);
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
		}

		// only the lower 5 bits of rs are used
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 3, 2, 1, 0, cpu_special_funcs::SLLV);

			// rs
			cpu->register_file.set_register(3, 0xFFFFFFE0);
			// rt
			cpu->register_file.set_register(2, 0xFFFF);
			// rd
			cpu->register_file.set_register(1, 0x0);

			cpu->execute(instruction);

			// no shifting should have occurred since the lower 5 bits were 0
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF);
		}
	}

	// Shift Word Right Logical Variable
	// SRLV rd, rt, rs
	// rd = rd >> rs
	SECTION("SRLV")
	{
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 3, 2, 1, 0, cpu_special_funcs::SRLV);

			// rs
			cpu->register_file.set_register(3, 16);
			// rt
			cpu->register_file.set_register(2, 0xFFFF0000);
			// rd
			cpu->register_file.set_register(1, 0x0);

			cpu->execute(instruction);
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF);
		}

		// only the lower 5 bits of rs are used
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 3, 2, 1, 0, cpu_special_funcs::SRLV
			);

			// rs
			cpu->register_file.set_register(3, 0xFFFFFFE0);
			// rt
			cpu->register_file.set_register(2, 0xFFFF0000);
			// rd
			cpu->register_file.set_register(1, 0x0);

			cpu->execute(instruction);

			// no shifting should have occurred since the lower 5 bits were 0
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
		}
	}

	SECTION("SRAV")
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

	SECTION("SYSCALL")
	{

	}

	SECTION("BREAK")
	{

	}
}

TEST_CASE("Special branching opcodes")
{
	SECTION("JR")
	{

	}

	SECTION("JALR")
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