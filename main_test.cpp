#define CATCH_CONFIG_MAIN
#define TESTING

#include <catch.hpp>
#include <string>
#include <memory>

#include "Ram.hpp"
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

	// make sure the cache isn't isolated
	Cop0::status_register status = cpu->cop0->get<Cop0::status_register>();
	status.Isc = false;
	cpu->cop0->set<Cop0::status_register>(status);


	SECTION("Load delay")
	{
		cpu->register_file.reset();
		ram->store_word(0x0, 0x1);

		// load value 0x1 from address 0x0 into register 1
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::LW);
			instr.immediate_instruction.immediate = 0;
			instr.immediate_instruction.rt = 1;

			cpu->execute(instr);

			REQUIRE(cpu->register_file.get_register(1) != 0x1);

			cpu->register_file.tick();
		}
		
		// nop
		{

			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::ADD);
			instr.register_instruction.rd = 0;
			instr.register_instruction.rt = 0;
			instr.register_instruction.rs = 0;

			cpu->execute(instr);

			REQUIRE(cpu->register_file.get_register(1) != 0x1);

			cpu->register_file.tick();
		}
		
		// on third instruction value is present
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDI);
			instr.immediate_instruction.immediate = 1;
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 1;

			// 1 in register now so adding 1 will result in 2
			cpu->execute(instr);

			REQUIRE(cpu->register_file.get_register(1) == 0x2);
		}
	}

	SECTION("Load delay with overlay")
	{
		cpu->register_file.reset();
		ram->store_word(0x0, 0x1);

		// load value 0x1 from address 0x0 into register 1
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::LW);
			instr.immediate_instruction.immediate = 0;
			instr.immediate_instruction.rt = 1;

			cpu->execute(instr);

			REQUIRE(cpu->register_file.get_register(1) != 0x1);

			cpu->register_file.tick();
		}

		// overwrite register 1 with 0 + 0
		{

			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::ADD);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rt = 0;
			instr.register_instruction.rs = 0;

			cpu->execute(instr);

			REQUIRE(cpu->register_file.get_register(1) != 0x1);

			cpu->register_file.tick();
		}

		// on third instruction value is present
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDI);
			instr.immediate_instruction.immediate = 1;
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 1;

			// 1 loaded by now but it was overwritten by 0 in the previous instruction
			cpu->execute(instr);

			REQUIRE(cpu->register_file.get_register(1) == 0x1);
		}
	}

	SECTION("r0 is always zero")
	{
		cpu->reset();
		REQUIRE(cpu->register_file.get_register(0) == 0x0);
		cpu->register_file.set_register(0, 0xDEADBEEF);
		REQUIRE(cpu->register_file.get_register(0) == 0x0);
	}

	SECTION("ensure r1 - r31 work")
	{
		cpu->reset();
		for (int index = 1; index < 32; index++)
		{
			REQUIRE(cpu->register_file.get_register(index) == 0x0);
			unsigned int value = 1 << (index - 1);
			cpu->register_file.set_register(index, value);
			REQUIRE(cpu->register_file.get_register(index) == value);
		}
	}

	SECTION("little endian load/store")
	{
		cpu->reset();
		REQUIRE(ram->load_word(0) == 0x0);
		REQUIRE(ram->load_halfword(0) == 0x0);
		REQUIRE(ram->load_byte(0) == 0x0);

		ram->store_word(0, 0xDEADBEEF);
		REQUIRE(ram->load_word(0) == 0xDEADBEEF);

		REQUIRE(ram->load_byte(0) == 0xEF);
		REQUIRE(ram->load_byte(1) == 0xBE);
		REQUIRE(ram->load_byte(2) == 0xAD);
		REQUIRE(ram->load_byte(3) == 0xDE);

		REQUIRE(ram->load_halfword(0) == 0xBEEF);
		REQUIRE(ram->load_halfword(2) == 0xDEAD);

		ram->store_halfword(0, 0xDEAD);
		REQUIRE(ram->load_word(0) == 0xDEADDEAD);

		ram->store_byte(0, 0xEE);
		REQUIRE(ram->load_word(0) == 0xDEADDEEE);
	}

	SECTION("ALU immediate")
	{
		// add immediate [rt 1] = [rs 2] + immediate
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDI);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;

			// simple addition
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 0xFFFFFFFE);

				instr.immediate_instruction.immediate = 0x1;
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFF);
			}

			// simple addition of negative immediate
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 0xFFFFFFFF);

				instr.immediate_instruction.immediate = static_cast<short>(-0x1);
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFE);
			}

			// overflow exception thrown
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);

				instr.immediate_instruction.immediate = 1;

				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);
			}

			// overflow exception thrown
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);

				instr.immediate_instruction.immediate = -1;

				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);
			}
		}

		// add immediate unsigned [rt 1] = [rs 2] + immediate
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ADDIU);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;
			// simple addition
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 0xFFFFFFFE);

				instr.immediate_instruction.immediate = 0x1;
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFF);
			}

			// simple addition of negative immediate
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 0xFFFFFFFF);

				instr.immediate_instruction.immediate = static_cast<short>(-0x1);
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFE);
			}

			// overflow exception not thrown
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);

				instr.immediate_instruction.immediate = 1;

				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}

			// overflow exception not thrown
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);
				instr.immediate_instruction.immediate = -1;

				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}
		}

		// set on less than immediate [rt 1] = 1 if [rs 2] < immediate else 0
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::SLTI);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 0x10);

				instr.immediate_instruction.immediate = 0x11;
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 1);

				instr.immediate_instruction.immediate = 0x09;
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0);
			}

			// extremes
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);

				instr.immediate_instruction.immediate = 0;
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 0);

				cpu->register_file.set_register(2, INT_MIN);
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 1);

				cpu->register_file.set_register(2, 0x0);
				instr.immediate_instruction.immediate = SHRT_MIN;
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 0);

				instr.immediate_instruction.immediate = SHRT_MAX;
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 1);
			}
		}

		// set on less than unsigned immediate [rt 1] = 1 if [rs 2] < immediate else 0
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::SLTIU);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;
			// unsigned so INT_MIN will actually be a large positive value
			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);

				instr.immediate_instruction.immediate = 0x0;
				cpu->execute(instr.raw);

				REQUIRE(cpu->register_file.get_register(1) == 0);
			}
		}

		// and immediate [rt 1] = [rs 2] & immediate
		{
			cpu->reset();

			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ANDI);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;
			instr.immediate_instruction.immediate = 0xFFFF;
			cpu->execute(instr.raw);

			REQUIRE(cpu->register_file.get_register(1) == 0x0);

			cpu->register_file.set_register(2, 0xFFFFFFFF);
			cpu->execute(instr.raw);
			REQUIRE(cpu->register_file.get_register(1) == 0x0000FFFF);

			cpu->register_file.set_register(2, 0xFFFFFFFF);
			instr.immediate_instruction.immediate = 0x0000;
			cpu->execute(instr.raw);
			REQUIRE(cpu->register_file.get_register(1) == 0x00000000);
		}

		// or immediate [rt 1] = [rs 2] | immediate
		{
			cpu->reset();

			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::ORI);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;
			instr.immediate_instruction.immediate = 0xFFFF;
			cpu->execute(instr.raw);

			REQUIRE(cpu->register_file.get_register(1) == 0x0000FFFF);

			cpu->register_file.set_register(2, 0xFFFF0000);
			cpu->execute(instr.raw);

			REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFF);
		}

		// xor immediate [rt 1] = [rs 2] ^ immediate
		{
			cpu->reset();

			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::XORI);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;
			instr.immediate_instruction.immediate = 0xFFFF;
			cpu->execute(instr.raw);

			REQUIRE(cpu->register_file.get_register(1) == 0x0000FFFF);

			cpu->register_file.set_register(2, 0xFFFFFFFF);
			cpu->execute(instr.raw);

			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
		}

		// lui [rt 1] = immediate << 16
		{
			cpu->reset();

			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::LUI);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.immediate = 0xFFFF;
			cpu->execute(instr.raw);

			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);

			cpu->register_file.set_register(2, 0x0000FFFF);
			cpu->execute(instr.raw);

			// the lower half of the word is not preserved
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
		}
	}

	SECTION("ALU registers")
	{
		/*
	void nor(const instruction_union& instr);
		*/

		// add [rd 1] = [rs 2] + [rt 3]
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::ADD);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 400);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == -200);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, -100);
				cpu->register_file.set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 200);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);
				cpu->register_file.set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == INT_MAX);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);
				cpu->register_file.set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == INT_MIN);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);
				cpu->register_file.set_register(3, INT_MAX);
				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);
				cpu->register_file.set_register(3, INT_MIN);
				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);
			}
		}

		// add unsigned [rd 1] = [rs 2] + [rt 3]
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::ADDU);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 400);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == -200);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, -100);
				cpu->register_file.set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 200);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);
				cpu->register_file.set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == INT_MAX);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);
				cpu->register_file.set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == INT_MIN);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);
				cpu->register_file.set_register(3, INT_MAX);
				REQUIRE_NOTHROW(cpu->execute(instr.raw));

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);
				cpu->register_file.set_register(3, INT_MIN);
				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}
		}

		// sub [rd 1] = [rs 2] - [rt 3]
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SUB);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 400);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, -100);
				cpu->register_file.set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == -400);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);
				cpu->register_file.set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == INT_MAX);

				// TODO - double check if this is correct!!!
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, 300);
				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);
				cpu->register_file.set_register(3, -1);
				REQUIRE_NOTHROW(cpu->execute(instr.raw));

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MIN);
				cpu->register_file.set_register(3, 1);
				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}
		}

		// sub unsigned [rd 1] = [rs 2] - [rt 3]
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SUBU);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 400);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, -100);
				cpu->register_file.set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == -400);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, INT_MAX);
				cpu->register_file.set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == INT_MAX);

				// TODO - double check if this is correct!!!
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, 300);
				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}
		}

		// slt [rd 1] = 1 if [rs 2] < [rt 3] else 0
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SLT);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, 200);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 1);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 200);
				cpu->register_file.set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, -200);
				cpu->register_file.set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 1);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, -100);
				cpu->register_file.set_register(3, 0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 1);
			}
		}

		// sltu [rd 1] = 1 if [rs 2] < [rt 3] else 0
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SLTU);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 100);
				cpu->register_file.set_register(3, 200);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 1);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, 200);
				cpu->register_file.set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0);

				cpu->register_file.set_register(1, 0x0); // result register
				cpu->register_file.set_register(2, -200);
				cpu->register_file.set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0);
			}
		}

		// and [rd 1] = [rs 2] & [rt 3]
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::AND);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(2, 0xDEAD0000);
				cpu->register_file.set_register(3, 0x0000BEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0x0);

				cpu->register_file.set_register(2, 0xDEADBEEF);
				cpu->register_file.set_register(3, 0x0000FFFF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0xBEEF);

				cpu->register_file.set_register(2, 0xDEADBEEF);
				cpu->register_file.set_register(3, 0xDEAD0000);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0xDEAD0000);
			}
		}

		// or [rd 1] = [rs 2] | [rt 3]
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::OR);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(2, 0xDEAD0000);
				cpu->register_file.set_register(3, 0x0000BEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0xDEADBEEF);

				cpu->register_file.set_register(2, 0xDEADBEEF);
				cpu->register_file.set_register(3, 0x0000FFFF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0xDEADFFFF);

				cpu->register_file.set_register(2, 0xDEADBEEF);
				cpu->register_file.set_register(3, 0xFFFF0000);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0xFFFFBEEF);
			}
		}

		// xor [rd 1] = [rs 2] ^ [rt 3]
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::XOR);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(2, 0xDEAD0000);
				cpu->register_file.set_register(3, 0x0000BEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0xDEADBEEF);

				cpu->register_file.set_register(2, 0xDEADBEEF);
				cpu->register_file.set_register(3, 0xDEADBEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0x0);
			}
		}

		// nor [rd 1] = !([rs 2] | [rt 3])
		{
			instruction_union instr;
			instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::NOR);
			instr.register_instruction.rd = 1;
			instr.register_instruction.rs = 2;
			instr.register_instruction.rt = 3;

			{
				cpu->register_file.set_register(2, 0x0);
				cpu->register_file.set_register(3, 0x0000FFFF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->register_file.get_register(1) == 0xFFFF0000);
			}
		}
	}

	SECTION("ALU shifting")
	{
		instruction_union instr;
		instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
		instr.register_instruction.rd = 1;
		instr.register_instruction.rt = 2;
		instr.register_instruction.rs = 3;
		//sll [rd 1] = [rt 2] << shamt
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SLL);
			instr.register_instruction.shamt = 16;
			cpu->register_file.set_register(2, 0xDEADBEEF);
			cpu->execute(instr.raw);

			REQUIRE(cpu->register_file.get_register(1) == 0xBEEF0000);
		}

		//srl [rd 1] = [rt 2] >> shamt
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRL);
			cpu->execute(instr.raw);
			REQUIRE(cpu->register_file.get_register(1) == 0x0000DEAD);
		}

		//sra [rd 1] = [rt 2] >> shamt
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRA);
			cpu->register_file.set_register(2, 0xFFFF0000);
			cpu->execute(instr.raw);
			REQUIRE(cpu->register_file.get_register(1) == 0xFFFFFFFF);
		}

		//sllv [rd 1] = [rt 2] << [rs 3]
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SLLV);
			cpu->register_file.set_register(2, 0xDEADBEEF);
			cpu->register_file.set_register(3, 8);
			cpu->execute(instr.raw);
			REQUIRE(cpu->register_file.get_register(1) == 0xADBEEF00);
		}

		//srlv [rd 1] = [rt 2] >> [rs 3]
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRLV);
			cpu->register_file.set_register(2, 0xDEADBEEF);
			cpu->register_file.set_register(3, 8);
			cpu->execute(instr.raw);
			REQUIRE(cpu->register_file.get_register(1) == 0x00DEADBE);
		}

		//srav [rd 1] = [rt 2] >> [rs 3]
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRAV);
			cpu->register_file.set_register(2, 0xF0000000);
			cpu->register_file.set_register(3, 8);
			cpu->execute(instr.raw);
			REQUIRE(cpu->register_file.get_register(1) == 0xFFF00000);
		}
	}

	SECTION("Multiply/Divide")
	{
		/*
		void mult(const instruction_union& instr);
	void mult_unsigned(const instruction_union& instr);
	void div(const instruction_union& instr);
	void div_unsigned(const instruction_union& instr);
	void move_from_hi(const instruction_union& instr);
	void move_from_lo(const instruction_union& instr);
	void move_to_hi(const instruction_union& instr);
	void move_to_lo(const instruction_union& instr);
		*/

		instruction_union instr;
		instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
		instr.register_instruction.rd = 1;
		instr.register_instruction.rt = 2;
		instr.register_instruction.rs = 3;
		// mult hi,lo = [rs 3] * [rt 2]
		{
			/*instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::MULT);
			cpu->register_file.set_register(3, INT_MAX);
			cpu->set_register(2, 4);
			cpu->execute(instr.raw);
			unsigned long long result = cpu->lo;
			result <<= 32;
			result |= cpu->hi;
			unsigned long long compare_result = INT_MAX;
			compare_result *= 4;
			REQUIRE(result == compare_result);*/
		}

		// multu hi,lo = rs * rt
		{

		}

		// div hi = remainder, lo = quotient
		{

		}

		// divu hi = remainer, lo = quotient
		{

		}
	}
}