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
		cpu->reset();
		REQUIRE(cpu->get_register(0) == 0x0);
		cpu->set_register(0, 0xDEADBEEF);
		REQUIRE(cpu->get_register(0) == 0x0);
	}

	SECTION("ensure r1 - r31 work")
	{
		cpu->reset();
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
		cpu->reset();
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFE);

				instr.immediate_instruction.immediate = 0x1;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFF);
			}

			// simple addition of negative immediate
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFF);

				instr.immediate_instruction.immediate = static_cast<short>(-0x1);
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFE);
			}

			// overflow exception thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);

				instr.immediate_instruction.immediate = 1;

				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);
			}

			// overflow exception thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);

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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFE);

				instr.immediate_instruction.immediate = 0x1;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFF);
			}

			// simple addition of negative immediate
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0xFFFFFFFF);

				instr.immediate_instruction.immediate = static_cast<short>(-0x1);
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0xFFFFFFFE);
			}

			// overflow exception not thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);

				instr.immediate_instruction.immediate = 1;

				REQUIRE_NOTHROW(cpu->execute(instr.raw));
			}

			// overflow exception not thrown
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 0x10);

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

		// set on less than unsigned immediate [rt 1] = 1 if [rs 2] < immediate else 0
		{
			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::SLTIU);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.rs = 2;
			// unsigned so INT_MIN will actually be a large positive value
			{
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);

				instr.immediate_instruction.immediate = 0x0;
				cpu->execute(instr.raw);

				REQUIRE(cpu->get_register(1) == 0);
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

			REQUIRE(cpu->get_register(1) == 0x0);

			cpu->set_register(2, 0xFFFFFFFF);
			cpu->execute(instr.raw);
			REQUIRE(cpu->get_register(1) == 0x0000FFFF);

			cpu->set_register(2, 0xFFFFFFFF);
			instr.immediate_instruction.immediate = 0x0000;
			cpu->execute(instr.raw);
			REQUIRE(cpu->get_register(1) == 0x00000000);
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

			REQUIRE(cpu->get_register(1) == 0x0000FFFF);

			cpu->set_register(2, 0xFFFF0000);
			cpu->execute(instr.raw);

			REQUIRE(cpu->get_register(1) == 0xFFFFFFFF);
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

			REQUIRE(cpu->get_register(1) == 0x0000FFFF);

			cpu->set_register(2, 0xFFFFFFFF);
			cpu->execute(instr.raw);

			REQUIRE(cpu->get_register(1) == 0xFFFF0000);
		}

		// lui [rt 1] = immediate << 16
		{
			cpu->reset();

			instruction_union instr;
			instr.immediate_instruction.op = static_cast<unsigned int>(cpu_instructions::LUI);
			instr.immediate_instruction.rt = 1;
			instr.immediate_instruction.immediate = 0xFFFF;
			cpu->execute(instr.raw);

			REQUIRE(cpu->get_register(1) == 0xFFFF0000);

			cpu->set_register(2, 0x0000FFFF);
			cpu->execute(instr.raw);

			// the lower half of the word is not preserved
			REQUIRE(cpu->get_register(1) == 0xFFFF0000);
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 400);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == -200);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, -100);
				cpu->set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 200);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);
				cpu->set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == INT_MAX);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);
				cpu->set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == INT_MIN);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);
				cpu->set_register(3, INT_MAX);
				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);
				cpu->set_register(3, INT_MIN);
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 400);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == -200);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, -100);
				cpu->set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 200);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);
				cpu->set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == INT_MAX);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);
				cpu->set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == INT_MIN);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);
				cpu->set_register(3, INT_MAX);
				REQUIRE_NOTHROW(cpu->execute(instr.raw));

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);
				cpu->set_register(3, INT_MIN);
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 400);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, -100);
				cpu->set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == -400);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);
				cpu->set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == INT_MAX);

				// TODO - double check if this is correct!!!
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, 300);
				REQUIRE_THROWS_AS(cpu->execute(instr.raw), overflow_exception);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);
				cpu->set_register(3, -1);
				REQUIRE_NOTHROW(cpu->execute(instr.raw));

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MIN);
				cpu->set_register(3, 1);
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, -300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 400);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, -100);
				cpu->set_register(3, 300);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == -400);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, INT_MAX);
				cpu->set_register(3, 0x0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == INT_MAX);

				// TODO - double check if this is correct!!!
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, 300);
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, 200);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 1);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 200);
				cpu->set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, -200);
				cpu->set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 1);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, -100);
				cpu->set_register(3, 0);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 1);
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
				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 100);
				cpu->set_register(3, 200);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 1);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, 200);
				cpu->set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0);

				cpu->set_register(1, 0x0); // result register
				cpu->set_register(2, -200);
				cpu->set_register(3, 100);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0);
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
				cpu->set_register(2, 0xDEAD0000);
				cpu->set_register(3, 0x0000BEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0x0);

				cpu->set_register(2, 0xDEADBEEF);
				cpu->set_register(3, 0x0000FFFF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0xBEEF);

				cpu->set_register(2, 0xDEADBEEF);
				cpu->set_register(3, 0xDEAD0000);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0xDEAD0000);
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
				cpu->set_register(2, 0xDEAD0000);
				cpu->set_register(3, 0x0000BEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0xDEADBEEF);

				cpu->set_register(2, 0xDEADBEEF);
				cpu->set_register(3, 0x0000FFFF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0xDEADFFFF);

				cpu->set_register(2, 0xDEADBEEF);
				cpu->set_register(3, 0xFFFF0000);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0xFFFFBEEF);
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
				cpu->set_register(2, 0xDEAD0000);
				cpu->set_register(3, 0x0000BEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0xDEADBEEF);

				cpu->set_register(2, 0xDEADBEEF);
				cpu->set_register(3, 0xDEADBEEF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0x0);
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
				cpu->set_register(2, 0x0);
				cpu->set_register(3, 0x0000FFFF);
				cpu->execute(instr.raw);
				REQUIRE(cpu->get_register(1) == 0xFFFF0000);
			}
		}
	}

	SECTION("ALU shifting")
	{
		/*
		void shift_right_arithmetic_variable(const instruction_union& instr);
		*/
		instruction_union instr;
		instr.register_instruction.op = static_cast<unsigned int>(cpu_instructions::SPECIAL);
		instr.register_instruction.rd = 1;
		instr.register_instruction.rt = 2;
		instr.register_instruction.rs = 3;
		//sll [rd 1] = [rt 2] << shamt
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SLL);
			instr.register_instruction.shamt = 16;
			cpu->set_register(2, 0xDEADBEEF);
			cpu->execute(instr.raw);

			REQUIRE(cpu->get_register(1) == 0xBEEF0000);
		}

		//srl [rd 1] = [rt 2] >> shamt
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRL);
			cpu->execute(instr.raw);
			REQUIRE(cpu->get_register(1) == 0x0000DEAD);
		}

		//sra [rd 1] = [rt 2] >> shamt
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRA);
			cpu->set_register(2, 0xFFFF0000);
			cpu->execute(instr.raw);
			REQUIRE(cpu->get_register(1) == 0xFFFFFFFF);
		}

		//sllv [rd 1] = [rt 2] << [rs 3]
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SLLV);
			cpu->set_register(2, 0xDEADBEEF);
			cpu->set_register(3, 8);
			cpu->execute(instr.raw);
			REQUIRE(cpu->get_register(1) == 0xADBEEF00);
		}

		//srlv [rd 1] = [rt 2] >> [rs 3]
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRLV);
			cpu->set_register(2, 0xDEADBEEF);
			cpu->set_register(3, 8);
			cpu->execute(instr.raw);
			REQUIRE(cpu->get_register(1) == 0x00DEADBE);
		}

		//srav [rd 1] = [rt 2] >> [rs 3]
		{
			instr.register_instruction.funct = static_cast<unsigned int>(cpu_special_funcs::SRAV);
			cpu->set_register(2, 0xF0000000);
			cpu->set_register(3, 8);
			cpu->execute(instr.raw);
			REQUIRE(cpu->get_register(1) == 0xFFF00000);
		}
	}
};