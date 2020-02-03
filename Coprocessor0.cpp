#include "Coprocessor0.hpp"
#include "InstructionTypes.hpp"
#include "InstructionEnums.hpp"
#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include <stdexcept>

Cop0::Cop0(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Cop(_ram, _cpu)
{
}

void Cop0::execute(const instruction_union& instruction)
{
	switch (static_cast<cpu_instructions>(instruction.immediate_instruction.op))
	{
		// LWC0
		case cpu_instructions::LWC0:
		{
			load_word_to_cop(instruction);
		} return;

		case cpu_instructions::SWC0:
		{
			store_word_from_cop(instruction);
		} return;
	}
	
	if (instruction.register_instruction.rs == 040)
	{
		cop0_instructions func = static_cast<cop0_instructions>(instruction.register_instruction.shamt);
		switch (func) {
			case cop0_instructions::TLBR:
			{
				read_indexed_tlb_entry(instruction);
			} break;
			case cop0_instructions::TLBWI:
			{
				write_indexed_tlb_entry(instruction);
			} break;
			case cop0_instructions::TLBWR:
			{
				write_random_tlb_entry(instruction);
			} break;
			case cop0_instructions::TLBP:
			{
				probe_tlb_for_matching_entry(instruction);
			} break;
			case cop0_instructions::RFE:
			{
				restore_from_exception(instruction);
			} break;
		}
	}
	else
	{
		copz_instructions function = static_cast<copz_instructions>(instruction.register_instruction.rs);
		switch (function) {
			case copz_instructions::MF:
			{
				move_from_cop(instruction);
			} break;

			case copz_instructions::CF:
			{
				move_control_from_cop(instruction);
			} break;

			case copz_instructions::MT:
			{
				move_to_cop(instruction);
			} break;

			case copz_instructions::CT:
			{
				move_control_to_cop(instruction);
			} break;
		}
	}
}

unsigned int Cop0::get_control_register(Cop0::register_names register_name)
{
	unsigned int index = static_cast<unsigned int>(register_name);
	return control_registers[index];
}

void Cop0::set_control_register(Cop0::register_names register_name, unsigned int value)
{
	unsigned int index = static_cast<unsigned int>(register_name);
	control_registers[index] = value;
}

unsigned int Cop0::get_control_register(unsigned int index)
{
	return control_registers[index];
}

void Cop0::set_control_register(unsigned int index, unsigned int value)
{
	control_registers[index] = value;
}

// LWCz rt, offset(base)
void Cop0::load_word_to_cop(const instruction_union& instr) 
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)cpu->get_register(instr.immediate_instruction.rs);
	unsigned int word = ram->load_word(addr);
	set_control_register(instr.immediate_instruction.rt,word);
}

// SWCz rt, offset(base)
void Cop0::store_word_from_cop(const instruction_union& instr)
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)cpu->get_register(instr.immediate_instruction.rs);
	ram->store_word(addr, get_control_register(instr.immediate_instruction.rt));
}

// MTCz rt, rd
void Cop0::move_to_cop(const instruction_union& instr)
{
	unsigned int value = cpu->get_register(instr.register_instruction.rt);
	set_control_register(instr.register_instruction.rd, value);
}

// MFCz rt, rd
void Cop0::move_from_cop(const instruction_union& instr)
{
	unsigned int value = get_control_register(instr.register_instruction.rd);
	cpu->set_register(instr.register_instruction.rt, value);
}

// CTCz rt, rd
void Cop0::move_control_to_cop(const instruction_union& instr)
{
	move_to_cop(instr);
}

// CFCz rt, rd
void Cop0::move_control_from_cop(const instruction_union& instr)
{
	move_from_cop(instr);
}

// COPz cofun
void Cop0::move_control_to_cop_fun(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Cop0::move_to_cp0(const instruction_union& instr)
{
	move_to_cop(instr);
}

void Cop0::move_from_cp0(const instruction_union& instr)
{
	move_from_cop(instr);
}

void Cop0::read_indexed_tlb_entry(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Cop0::write_indexed_tlb_entry(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Cop0::write_random_tlb_entry(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Cop0::probe_tlb_for_matching_entry(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Cop0::restore_from_exception(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}