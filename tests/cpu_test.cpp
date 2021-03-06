#include <catch.hpp>

#include <string>
#include <memory>
#include <limits>

#include "../Bus.hpp"
#include "../Ram.hpp"
#include "../Cpu.hpp"
#include "../SystemControlCoprocessor.hpp"
#include "../InstructionEnums.hpp"

TEST_CASE("Standard Opcodes")
{
	std::shared_ptr<Bus> bus = std::make_shared<Bus>();
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	bus->register_device(ram.get());

	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(bus);

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
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, std::numeric_limits<int>::max());
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::max());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == true);
			REQUIRE(cpu->pending_exception_excode == static_cast<unsigned int>(SystemControlCoprocessor::excode::Ov));
		}

		{
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, std::numeric_limits<int>::min());
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::min());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == true);
			REQUIRE(cpu->pending_exception_excode == static_cast<unsigned int>(SystemControlCoprocessor::excode::Ov));
		}

		{
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::max());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == false);
		}

		{
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDI, 1, 1, std::numeric_limits<short>::min());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == false);
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
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, std::numeric_limits<int>::max());
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::max());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == false);
		}

		{
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, std::numeric_limits<int>::min());
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::min());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == false);
		}

		{
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::max());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == false);
		}

		{
			cpu->pending_exception = false;
			cpu->register_file.set_register(1, 0);
			instruction_union instruction(cpu_instructions::ADDIU, 1, 1, std::numeric_limits<short>::min());
			cpu->execute(instruction);
			REQUIRE(cpu->pending_exception == false);
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
	std::shared_ptr<Bus> bus = std::make_shared<Bus>();
	std::shared_ptr<Ram> ram = std::make_shared<Ram>();
	bus->register_device(ram.get());

	std::shared_ptr<Cpu> cpu = std::make_shared<Cpu>();
	cpu->init(bus);

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

	// Shift Word Right Arithmetic Variable
	// SRAV rd, rt, rs
	SECTION("SRAV")
	{
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 3, 2, 1, 0, cpu_special_funcs::SRAV);

			// rs
			cpu->register_file.set_register(3, 16);
			// rt
			cpu->register_file.set_register(2, 0xFFFF0000);
			// rd
			cpu->register_file.set_register(1, 0x0);

			cpu->execute(instruction);
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFF);
		}

		// only lower 5 bits used
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 3, 2, 1, 0, cpu_special_funcs::SRAV);

			// rs
			cpu->register_file.set_register(3, 0xFFFFFFE0);
			// rt
			cpu->register_file.set_register(2, 0xFFFF0000);
			// rd
			cpu->register_file.set_register(1, 0x0);

			cpu->execute(instruction);
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
		}
	}

	// Move From Hi
	// MFHI rd
	// rd = hi
	SECTION("MFHI")
	{
		instruction_union instruction(cpu_instructions::SPECIAL, 0, 0, 1, 0, cpu_special_funcs::MFHI);

		cpu->hi = 0xF0F0F0F0;

		// rd
		cpu->register_file.set_register(1, 0x0);

		cpu->execute(instruction);
		REQUIRE(cpu->register_file.get_register(1) == 0xF0F0F0F0);
	}

	// Move To Hi
	// MTHI rs
	// hi = rs
	SECTION("MTHI")
	{
		instruction_union instruction(cpu_instructions::SPECIAL, 1, 0, 0, 0, cpu_special_funcs::MTHI);

		cpu->hi = 0x0;

		// rs
		cpu->register_file.set_register(1, 0x0F0F0F0F);

		cpu->execute(instruction);
		REQUIRE(cpu->hi == 0x0F0F0F0F);
	}

	// Move From Lo
	// MFLO rd
	// rd = lo
	SECTION("MFLO")
	{
		instruction_union instruction(cpu_instructions::SPECIAL, 0, 0, 1, 0, cpu_special_funcs::MFLO);

		cpu->lo = 0xF0F0F0F0;

		// rd
		cpu->register_file.set_register(1, 0x0);

		cpu->execute(instruction);
		REQUIRE(cpu->register_file.get_register(1) == 0xF0F0F0F0);
	}

	// Move To Lo
	// MTLO rs
	// lo = rs
	SECTION("MTLO")
	{
		instruction_union instruction(cpu_instructions::SPECIAL, 1, 0, 0, 0, cpu_special_funcs::MTLO);

		cpu->lo = 0x0;

		// rs
		cpu->register_file.set_register(1, 0x0F0F0F0F);

		cpu->execute(instruction);
		REQUIRE(cpu->lo == 0x0F0F0F0F);
	}


	// Multiply Word
	// MULT rs, rt
	SECTION("MULT")
	{
		// normal positive * positive value that takes up more than 32 bits
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 2, 1, 0, 0, cpu_special_funcs::MULT);

			// rs
			cpu->register_file.set_register(2, std::numeric_limits<int>::max());

			// rt
			cpu->register_file.set_register(1, 4);

			cpu->execute(instruction);

			long long result = ((long long)(cpu->hi) << 32) | cpu->lo;
			long long correct_result = std::numeric_limits<int>::max() * 4;

			REQUIRE(result == correct_result);
		}

		// normal positive * negtive value that takes up more than 32 bits
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 2, 1, 0, 0, cpu_special_funcs::MULT);

			// rs
			cpu->register_file.set_register(2, std::numeric_limits<int>::max());

			// rt
			cpu->register_file.set_register(1, -4);

			cpu->execute(instruction);

			long long result = ((long long)(cpu->hi) << 32) | cpu->lo;
			long long correct_result = std::numeric_limits<int>::max() * (-4);

			REQUIRE(result == correct_result);
		}
	}

	SECTION("MULTU")
	{
		// normal positive * positive value that takes up more than 32 bits
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 2, 1, 0, 0, cpu_special_funcs::MULTU);

			// rs
			cpu->register_file.set_register(2, std::numeric_limits<unsigned int>::max());

			// rt
			cpu->register_file.set_register(1, 4);

			cpu->execute(instruction);

			long long result = ((long long)(cpu->hi) << 32) | cpu->lo;
			long long correct_result = std::numeric_limits<unsigned int>::max() * 4;

			REQUIRE(result == correct_result);
		}
	}

	SECTION("DIV")
	{
		// normal positive * positive value that takes up more than 32 bits
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 2, 1, 0, 0, cpu_special_funcs::DIV);

			// rs
			cpu->register_file.set_register(2, std::numeric_limits<int>::max());

			// rt
			cpu->register_file.set_register(1, 4);

			cpu->execute(instruction);

			int result_quotient = cpu->lo;
			int result_remainder = cpu->hi;

			int correct_quotient = std::numeric_limits<int>::max() / 4;
			int correct_remainder = std::numeric_limits<int>::max() % 4;

			REQUIRE(result_quotient == correct_quotient);
			REQUIRE(result_remainder == correct_remainder);
		}
	}

	SECTION("DIVU")
	{
		// normal positive * positive value that takes up more than 32 bits
		{
			instruction_union instruction(cpu_instructions::SPECIAL, 2, 1, 0, 0, cpu_special_funcs::DIVU);

			// rs
			cpu->register_file.set_register(2, std::numeric_limits<unsigned int>::max());

			// rt
			cpu->register_file.set_register(1, 4);

			cpu->execute(instruction);

			unsigned int result_quotient = cpu->lo;
			unsigned int result_remainder = cpu->hi;

			unsigned int correct_quotient = std::numeric_limits<unsigned int>::max() / 4;
			unsigned int correct_remainder = std::numeric_limits<unsigned int>::max() % 4;

			REQUIRE(result_quotient == correct_quotient);
			REQUIRE(result_remainder == correct_remainder);
		}
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