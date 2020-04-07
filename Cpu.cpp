#include <assert.h>
#include <iostream>
#include <fstream>
#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "Coprocessor0.hpp"
#include "Coprocessor2.hpp"
#include "InstructionEnums.hpp"
#include "Exceptions.hpp"

void Cpu::init(std::shared_ptr<Ram> _ram)
{
	ram = _ram;
	cop0 = std::make_shared<Cop0>(ram, shared_from_this());
	cop2 = std::make_shared<Cop2>(ram, shared_from_this());
	reset();
}

void Cpu::reset()
{
	current_instruction = 0;
	next_instruction = 0;
	current_pc = static_cast<unsigned int>(Cop0::exception_vector::RESET);
	next_pc = current_pc;
	cop0->reset();
	cop2->reset();
	register_file.reset();
}

void Cpu::tick()
{
	current_pc = next_pc;
	current_instruction = next_instruction;

	next_instruction = ram->load_word(current_pc);
	next_pc = current_pc + 4;

	try
	{
		execute(current_instruction);
	}
	catch(sys_call& /*e*/)
	{
		Cop0::cause_register cause = cop0->get<Cop0::cause_register>();
		cop0->set_control_register(Cop0::register_names::EPC, current_pc);

		if (in_delay_slot) {
			cause.BD = true;
			cop0->set_control_register(Cop0::register_names::EPC, current_pc - 4);
		}

		Cop0::status_register sr = cop0->get<Cop0::status_register>();
		next_pc = static_cast<unsigned int>(sr.BEV == 0 ? Cop0::exception_vector::GENERAL_BEV0 : Cop0::exception_vector::GENERAL_BEV1);

		unsigned int mode = sr.raw & 0x3f;
		sr.raw &= ~0x3f;
		sr.raw |= (mode << 2) & 0x3f;

		cop0->set<Cop0::status_register>(sr);
		
		cause.Excode = static_cast<unsigned int>(Cop0::excode::Syscall);
		cop0->set<Cop0::cause_register>(cause);

		// dump the next instruction
		next_instruction = 0x0;
	}
	catch (...)
	{
		throw std::logic_error("not implemented yet");
	}

	register_file.merge();
	in_delay_slot = false;
}


void Cpu::save_state(std::ofstream& file)
{
	cop0->save_state(file);
	cop2->save_state(file);

	file.write(reinterpret_cast<char*>(&register_file.gp_registers[0]), sizeof(unsigned int)*32);
	file.write(reinterpret_cast<char*>(&register_file.shadow_gp_registers_first[0]), sizeof(unsigned int) * 32);
	file.write(reinterpret_cast<char*>(&register_file.shadow_gp_registers_last[0]), sizeof(unsigned int) * 32);

	file.write(reinterpret_cast<char*>(&hi), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&lo), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&current_pc), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&next_pc), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&current_instruction), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&next_instruction), sizeof(unsigned int));
	file.write(reinterpret_cast<char*>(&in_delay_slot), sizeof(bool));
}

void Cpu::load_state(std::ifstream& file)
{
	cop0->load_state(file);
	cop2->load_state(file);

	file.read(reinterpret_cast<char*>(&register_file.gp_registers[0]), sizeof(unsigned int) * 32);
	file.read(reinterpret_cast<char*>(&register_file.shadow_gp_registers_first[0]), sizeof(unsigned int) * 32);
	file.read(reinterpret_cast<char*>(&register_file.shadow_gp_registers_last[0]), sizeof(unsigned int) * 32);

	file.read(reinterpret_cast<char*>(&hi), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&lo), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&current_pc), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&next_pc), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&current_instruction), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&next_instruction), sizeof(unsigned int));
	file.read(reinterpret_cast<char*>(&in_delay_slot), sizeof(bool));
}

void Cpu::execute(unsigned int instruction)
{
	instruction_union instr;
	instr.raw = instruction;

	cpu_instructions opcode = static_cast<cpu_instructions>(instruction >> 26);

	switch (opcode)
	{
		case cpu_instructions::ADDI:
		{
			int immediate = (short)instr.immediate_instruction.immediate;
			int rs_value = get_register(instr.immediate_instruction.rs);

			unsigned int value = rs_value + immediate;

			// check for overflow
			{
				int signed_value = value;
				if (immediate >= 0 && rs_value >= 0)
				{
					if (signed_value < 0)
					{
						throw overflow_exception();
					}
				}
				else if (immediate < 0 && rs_value < 0)
				{
					if (signed_value >= 0)
					{
						throw overflow_exception();
					}
				}
			}

			set_register(instr.immediate_instruction.rt, value);
		} break;

		case cpu_instructions::ADDIU:
		{
			int immediate = (short)instr.immediate_instruction.immediate;
			int rs_value = get_register(instr.immediate_instruction.rs);

			unsigned int value = rs_value + immediate;

			set_register(instr.immediate_instruction.rt, value);
		} break;

		case cpu_instructions::ANDI:
		{
			unsigned int value = instr.immediate_instruction.immediate & get_register(instr.immediate_instruction.rs);
			set_register(instr.immediate_instruction.rt, value);
		} break;

		case cpu_instructions::BCOND:
		{
			execute_bcond(instr);
		} break;

		case cpu_instructions::BEQ:
		{
			unsigned int rs_value = get_register(instr.immediate_instruction.rs);
			unsigned int rt_value = get_register(instr.immediate_instruction.rt);
			if (rs_value == rt_value)
			{
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;

		case cpu_instructions::BGTZ:
		{
			int rs_value = get_register(instr.immediate_instruction.rs);
			if (rs_value > 0)
			{
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;
		
		case cpu_instructions::BLEZ:
		{
			int rs_value = get_register(instr.immediate_instruction.rs);
			if (rs_value <= 0)
			{
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;

		case cpu_instructions::BNE:
		{
			unsigned int rs_value = get_register(instr.immediate_instruction.rs);
			unsigned int rt_value = get_register(instr.immediate_instruction.rt);
			if (rs_value != rt_value)
			{
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;

		case cpu_instructions::SWC0:
		case cpu_instructions::SWC2:
		case cpu_instructions::LWC0:
		case cpu_instructions::LWC2:
		case cpu_instructions::COP2:
		case cpu_instructions::COP0:
		{
			execute_cop(instr);
		} break;

		case cpu_instructions::J:
		{
			unsigned int target = instr.jump_instruction.target << 2;

			next_pc = target | (0xF0000000 & next_pc);

			in_delay_slot = true;
		} break;

		case cpu_instructions::JAL:
		{
			set_register(31, next_pc);
			unsigned int target = instr.jump_instruction.target << 2;

			next_pc = target | (0xF0000000 & next_pc);

			in_delay_slot = true;
		} break;

		case cpu_instructions::LB:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned char value = ram->load_byte(addr);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				set_register(instr.immediate_instruction.rt, value, true);
			}
		} break;

		case cpu_instructions::LBU:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			int value = (char)ram->load_byte(addr);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				set_register(instr.immediate_instruction.rt, value, true);
			}
		} break;

		case cpu_instructions::LH:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			int value = (short)ram->load_halfword(addr);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				set_register(instr.immediate_instruction.rt, value, true);
			}
		} break;

		case cpu_instructions::LHU:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned short value = ram->load_halfword(addr);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				set_register(instr.immediate_instruction.rt, value, true);
			}
		} break;

		case cpu_instructions::LUI:
		{
			unsigned int value = instr.immediate_instruction.immediate << 16;
			set_register(instr.immediate_instruction.rt, value);
		} break;

		case cpu_instructions::LW:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int value = ram->load_word(addr);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				set_register(instr.immediate_instruction.rt, value, true);
			}
		} break;

		case cpu_instructions::LWL:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int addr_aligned = addr & ~3;
			unsigned int aligned_value = ram->load_word(addr_aligned);

			// can load on top of value currently in the pipeline
			unsigned int current_value = register_file.shadow_gp_registers_first[instr.immediate_instruction.rt];

			unsigned int alignment = addr & 3;
			unsigned int mask = 0x00ffffff >> (alignment * 8);
			unsigned int new_value = (current_value & mask) | (aligned_value << ((3 - alignment) * 8));
			set_register(instr.immediate_instruction.rt, new_value);
		} break;

		case cpu_instructions::LWR:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int addr_aligned = addr & ~3;
			unsigned int aligned_value = ram->load_word(addr_aligned);

			// can load on top of value currently in the pipeline
			unsigned int current_value = register_file.shadow_gp_registers_first[instr.immediate_instruction.rt];

			unsigned int alignment = addr & 3;
			unsigned int mask = 0xffffff00 << ((3 - alignment) * 8);
			unsigned int new_value = (current_value & mask) | (aligned_value >> alignment * 8);
			set_register(instr.immediate_instruction.rt, new_value);
		} break;

		case cpu_instructions::ORI:
		{
			unsigned int rs_value = get_register(instr.immediate_instruction.rs);
			unsigned int value = rs_value | instr.immediate_instruction.immediate;
			set_register(instr.immediate_instruction.rt, value);
		} break;

		case cpu_instructions::SB:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int value = get_register(instr.immediate_instruction.rt);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				ram->store_byte(addr, value);
			}
		} break;

		case cpu_instructions::SH:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int value = get_register(instr.immediate_instruction.rt);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				ram->store_halfword(addr, value);
			}
		} break;

		case cpu_instructions::SLTI:
		{
			int rs_value = get_register(instr.immediate_instruction.rs);
			int immediate_value = (short)instr.immediate_instruction.immediate;

			set_register(instr.immediate_instruction.rt, rs_value < immediate_value ? 1 : 0);
		} break;

		case cpu_instructions::SLTIU:
		{
			unsigned int rs_value = get_register(instr.immediate_instruction.rs);
			int immediate_value = (short)instr.immediate_instruction.immediate;

			set_register(instr.immediate_instruction.rt, rs_value < (unsigned int)immediate_value ? 1 : 0);
		} break;

		case cpu_instructions::SPECIAL:
		{
			execute_special(instr);
		} break;

		case cpu_instructions::SW:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int value = get_register(instr.immediate_instruction.rt);

			if (cop0->get<Cop0::status_register>().Isc == false)
			{
				ram->store_word(addr, value);
			}
		} break;

		case cpu_instructions::SWL:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int addr_aligned = addr & ~3;
			unsigned int aligned_value = ram->load_word(addr_aligned);
			unsigned int value_to_set = get_register(instr.immediate_instruction.rt);

			unsigned int alignment = addr & 3;
			unsigned int mask = 0xffffff00 << (alignment * 8);

			unsigned int new_value = (aligned_value & mask) | (value_to_set >> ((3 - alignment) * 8));
			ram->store_word(addr_aligned, new_value);
		} break;

		case cpu_instructions::SWR:
		{
			unsigned int addr = get_immediate_base_addr(instr);
			unsigned int addr_aligned = addr & ~3;
			unsigned int aligned_value = ram->load_word(addr_aligned);
			unsigned int value_to_set = get_register(instr.immediate_instruction.rt);

			unsigned int alignment = addr & 3;
			unsigned int mask = 0x00ffffff >> ((3 - alignment) * 8);

			unsigned int new_value = (aligned_value & mask) | (value_to_set << (alignment * 8));
			ram->store_word(addr_aligned, new_value);
		} break;

		case cpu_instructions::XORI:
		{
			unsigned int rs_value = get_register(instr.immediate_instruction.rs);
			unsigned int value = rs_value ^ instr.immediate_instruction.immediate;
			set_register(instr.immediate_instruction.rt, value);
		} break;
	}
}

void Cpu::execute_special(const instruction_union& instr)
{
	cpu_special_funcs func = static_cast<cpu_special_funcs>(instr.register_instruction.funct);
	switch (func)
	{
		case cpu_special_funcs::ADD:
		{
			int rs_value = get_register(instr.register_instruction.rs);
			int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rs_value + rt_value;

			// check for overflow
			{
				int signed_value = value;
				if (rt_value >= 0 && rs_value >= 0)
				{
					if (signed_value < 0)
					{
						throw overflow_exception();
					}
				}
				else if (rt_value < 0 && rs_value < 0)
				{
					if (signed_value >= 0)
					{
						throw overflow_exception();
					}
				}
			}

			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::ADDU:
		{
			int rs_value = get_register(instr.register_instruction.rs);
			int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rs_value + rt_value;

			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::AND:
		{
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rs_value & rt_value;
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::BREAK:
		{
			throw breakpoint_exception();
		} break;

		case cpu_special_funcs::DIV:
		{
			int rs_value = get_register(instr.register_instruction.rs);
			int rt_value = get_register(instr.register_instruction.rt);

			if (rt_value == 0)
			{
				hi = rs_value;
				if (rs_value >= 0)
				{
					lo = 0xffffffff;
				}
				else
				{
					lo = 1;
				}
			}
			else if (rs_value == 0x80000000 && rt_value == -1)
			{
				hi = 0;
				lo = 0x80000000;
			}
			else
			{
				hi = rs_value % rt_value;
				lo = rs_value / rt_value;
			}
		} break;

		case cpu_special_funcs::DIVU:
		{
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int rt_value = get_register(instr.register_instruction.rt);

			if (rt_value == 0)
			{
				hi = rs_value;
				lo = 0xffffffff;
			}
			else
			{
				hi = rs_value % rt_value;
				lo = rs_value / rt_value;
			}
		} break;

		case cpu_special_funcs::JALR:
		{
			set_register(instr.register_instruction.rd, next_pc);
			next_pc = get_register(instr.register_instruction.rs);
			in_delay_slot = true;
		} break;

		case cpu_special_funcs::JR:
		{
			next_pc = get_register(instr.register_instruction.rs);
			in_delay_slot = true;
		} break;

		case cpu_special_funcs::MFHI:
		{
			set_register(instr.register_instruction.rd, hi);
		} break;

		case cpu_special_funcs::MFLO:
		{
			set_register(instr.register_instruction.rd, lo);
		} break;

		case cpu_special_funcs::MTHI:
		{
			hi = get_register(instr.register_instruction.rs);
		} break;

		case cpu_special_funcs::MTLO:
		{
			lo = get_register(instr.register_instruction.rs);
		} break;

		case cpu_special_funcs::MULT:
		{
			int rs_value = get_register(instr.register_instruction.rs);
			int rt_value = get_register(instr.register_instruction.rt);
			long long result = rs_value * rt_value;
			hi = result >> 32;
			lo = result & 0xFFFF;
		} break;

		case cpu_special_funcs::MULTU:
		{
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned long long result = rs_value * rt_value;
			hi = result >> 32;
			lo = result & 0xFFFF;
		} break;

		case cpu_special_funcs::NOR:
		{
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = ~(rs_value | rt_value);
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::OR:
		{
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rs_value | rt_value;
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SLL:
		{
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rt_value << instr.register_instruction.shamt;
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SLLV:
		{
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			// restrict shift values to lower 5 bits so shifts over 32 bits, result in effectively a NOP instruction
			unsigned int value = rt_value << (0x1F & rs_value);

			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SLT:
		{
			int rt_value = get_register(instr.register_instruction.rt);
			int rs_value = get_register(instr.register_instruction.rs);

			set_register(instr.register_instruction.rd, rs_value < rt_value ? 1 : 0);
		} break;

		case cpu_special_funcs::SLTU:
		{
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int rs_value = get_register(instr.register_instruction.rs);

			set_register(instr.register_instruction.rd, rs_value < rt_value ? 1 : 0);
		} break;

		case cpu_special_funcs::SRA:
		{
			int rt_value = (int)(get_register(instr.register_instruction.rt));
			unsigned int value = rt_value >> instr.register_instruction.shamt;
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SRAV:
		{
			int rt_value = get_register(instr.register_instruction.rt);
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int value = rt_value >> (0x1F & rs_value);
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SRL:
		{
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rt_value >> instr.register_instruction.shamt;
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SRLV:
		{
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int value = rt_value >> (0x1F & rs_value);
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SUB:
		{
			int rs_value = get_register(instr.register_instruction.rs);
			int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rs_value - rt_value;

			// check for overflow
			{
				int signed_value = value;
				if (rt_value >= 0 && rs_value >= 0)
				{
					if (signed_value < 0)
					{
						throw overflow_exception();
					}
				}
				else if (rt_value < 0 && rs_value < 0)
				{
					if (signed_value >= 0)
					{
						throw overflow_exception();
					}
				}
			}

			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SUBU:
		{
			int rs_value = get_register(instr.register_instruction.rs);
			int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rs_value - rt_value;
			set_register(instr.register_instruction.rd, value);
		} break;

		case cpu_special_funcs::SYSCALL:
		{
			throw sys_call();
		} break;

		case cpu_special_funcs::XOR:
		{
			unsigned int rs_value = get_register(instr.register_instruction.rs);
			unsigned int rt_value = get_register(instr.register_instruction.rt);
			unsigned int value = rs_value ^ rt_value;
			set_register(instr.register_instruction.rd, value);
		} break;
	}
}

void Cpu::execute_bcond(const instruction_union& instr)
{
	cpu_bconds cond = static_cast<cpu_bconds>(instr.immediate_instruction.rt);
	switch (cond)
	{
		case cpu_bconds::BGEZ:
		{
			int rs_value = get_register(instr.immediate_instruction.rs);
			if (rs_value >= 0)
			{
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;

		case cpu_bconds::BGEZAL:
		{
			int rs_value = get_register(instr.immediate_instruction.rs);
			if (rs_value >= 0)
			{
				set_register(instr.register_instruction.rd, next_pc);
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;

		case cpu_bconds::BLTZ:
		{
			int rs_value = get_register(instr.immediate_instruction.rs);
			if (rs_value < 0)
			{
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;

		case cpu_bconds::BLTZAL:
		{
			int rs_value = get_register(instr.immediate_instruction.rs);
			if (rs_value < 0)
			{
				set_register(instr.register_instruction.rd, next_pc);
				unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
				next_pc += offset;
				next_pc -= 4;
				in_delay_slot = true;
			}
		} break;
	}
}

void Cpu::execute_cop(const instruction_union& instr)
{
	unsigned int cop_number = (instr.raw >> 26) & 0x3;
	if (cop_number == 0)
	{
		cop0->execute(instr);
	}
	else if (cop_number == 1)
	{
		cop2->execute(instr);
	}
}

unsigned int Cpu::get_register(int index) 
{
	unsigned int result = register_file.gp_registers[index];
	return result;
}

void Cpu::set_register(int index, unsigned int value, bool load_delay) 
{
	if (index != 0)
	{
		if (load_delay)
		{
			register_file.shadow_gp_registers_first[index] = value;
		}
		else
		{
			register_file.gp_registers[index] = value;
			register_file.shadow_gp_registers_first[index] = value;
			register_file.shadow_gp_registers_last[index] = value;
		}
	}
}

unsigned int Cpu::get_immediate_base_addr(const instruction_union& instr)
{
	int offset = (short)instr.immediate_instruction.immediate;
	int base = get_register(instr.immediate_instruction.rs);

	unsigned int addr = offset + base;
	return addr;
}