#include "Coprocessor0.hpp"
#include "InstructionTypes.hpp"
#include "Ram.hpp"
#include "Cpu.hpp"
#include <stdexcept>

enum class cop0 : unsigned char
{
	TLBR = 001,
	TLBWI = 002,
	TLBWR = 006,
	TLBP = 010,
	RFE = 020
};

Coprocessor0::Coprocessor0(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Coprocessor(_ram, _cpu)
{
}

void Coprocessor0::execute(unsigned int instruction)
{
	ImmediateInstruction imm_instr(instruction);
	switch (imm_instr.op)
	{
		// LWC0
		case 060:
		{
			load_word_to_cop(imm_instr);
		} return;

		// SWC0
		case 070:
		{
			store_word_from_cop(imm_instr);
		} return;
	}

	RegisterInstruction reg_instr(instruction);
	
	if (reg_instr.rs == 040)
	{
		cop0 func = static_cast<cop0>(reg_instr.shamt);
		switch (func) {
			case cop0::TLBR:
			{
				read_indexed_tlb_entry(reg_instr);
			} break;
			case cop0::TLBWI:
			{
				write_indexed_tlb_entry(reg_instr);
			} break;
			case cop0::TLBWR:
			{
				write_random_tlb_entry(reg_instr);
			} break;
			case cop0::TLBP:
			{
				probe_tlb_for_matching_entry(reg_instr);
			} break;
			case cop0::RFE:
			{
				restore_from_exception(reg_instr);
			} break;
		}
	}
	else
	{
		copz function = static_cast<copz>(reg_instr.rs);
		switch (function) {
			case copz::MF:
			{
				move_from_cop(reg_instr);
			} break;

			case copz::CF:
			{
				move_control_from_cop(reg_instr);
			} break;

			case copz::MT:
			{
				move_to_cop(reg_instr);
			} break;

			case copz::CT:
			{
				move_control_to_cop(reg_instr);
			} break;
		}
	}
}

unsigned int Coprocessor0::get_control_register(unsigned int index)
{
	return control_registers[index];
}

void Coprocessor0::set_control_register(unsigned int index, unsigned int value)
{
	control_registers[index] = value;
}

// LWCz rt, offset(base)
void Coprocessor0::load_word_to_cop(const ImmediateInstruction& instr) 
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	unsigned int *word = ram->get_word(addr);
	set_control_register(instr.rt,*word);
}

// SWCz rt, offset(base)
void Coprocessor0::store_word_from_cop(const ImmediateInstruction& instr) 
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	unsigned int *word = ram->get_word(addr);
	*word = get_control_register(instr.rt);
}

// MTCz rt, rd
void Coprocessor0::move_to_cop(const RegisterInstruction& instr) 
{
	unsigned int value = cpu->get_register(instr.rt);
	set_control_register(instr.rs, value);
}

// MFCz rt, rd
void Coprocessor0::move_from_cop(const RegisterInstruction& instr) 
{
	unsigned int value = get_control_register(instr.rd);
	cpu->set_register(instr.rt, value);
}

// CTCz rt, rd
void Coprocessor0::move_control_to_cop(const RegisterInstruction& instr) 
{
	move_to_cop(instr);
}

// CFCz rt, rd
void Coprocessor0::move_control_from_cop(const RegisterInstruction& instr) 
{
	move_from_cop(instr);
}

// COPz cofun
void Coprocessor0::move_control_to_cop_fun(const RegisterInstruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::move_to_cp0(const RegisterInstruction& instr)
{
	move_to_cop(instr);
}

void Coprocessor0::move_from_cp0(const RegisterInstruction& instr)
{
	move_from_cop(instr);
}

void Coprocessor0::read_indexed_tlb_entry(const RegisterInstruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::write_indexed_tlb_entry(const RegisterInstruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::write_random_tlb_entry(const RegisterInstruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::probe_tlb_for_matching_entry(const RegisterInstruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::restore_from_exception(const RegisterInstruction& instr)
{
	throw std::logic_error("not supported on cop0");
}