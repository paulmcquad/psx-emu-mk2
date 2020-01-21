#include "Coprocessor0.hpp"
#include "InstructionTypes.hpp"
#include "InstructionEnums.hpp"
#include "Ram.hpp"
#include "Cpu.hpp"
#include <stdexcept>

Coprocessor0::Coprocessor0(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Coprocessor(_ram, _cpu)
{
}

void Coprocessor0::execute(unsigned int instruction)
{
	immediate_instruction imm_instr(instruction);
	switch (static_cast<cpu_instructions>(imm_instr.op))
	{
		// LWC0
		case cpu_instructions::LWC0:
		{
			load_word_to_cop(imm_instr);
		} return;

		case cpu_instructions::SWC0:
		{
			store_word_from_cop(imm_instr);
		} return;
	}

	register_instruction reg_instr(instruction);
	
	if (reg_instr.rs == 040)
	{
		cop0_instructions func = static_cast<cop0_instructions>(reg_instr.shamt);
		switch (func) {
			case cop0_instructions::TLBR:
			{
				read_indexed_tlb_entry(reg_instr);
			} break;
			case cop0_instructions::TLBWI:
			{
				write_indexed_tlb_entry(reg_instr);
			} break;
			case cop0_instructions::TLBWR:
			{
				write_random_tlb_entry(reg_instr);
			} break;
			case cop0_instructions::TLBP:
			{
				probe_tlb_for_matching_entry(reg_instr);
			} break;
			case cop0_instructions::RFE:
			{
				restore_from_exception(reg_instr);
			} break;
		}
	}
	else
	{
		copz_instructions function = static_cast<copz_instructions>(reg_instr.rs);
		switch (function) {
			case copz_instructions::MF:
			{
				move_from_cop(reg_instr);
			} break;

			case copz_instructions::CF:
			{
				move_control_from_cop(reg_instr);
			} break;

			case copz_instructions::MT:
			{
				move_to_cop(reg_instr);
			} break;

			case copz_instructions::CT:
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
void Coprocessor0::load_word_to_cop(const immediate_instruction& instr) 
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	unsigned int *word = ram->get_word(addr);
	set_control_register(instr.rt,*word);
}

// SWCz rt, offset(base)
void Coprocessor0::store_word_from_cop(const immediate_instruction& instr) 
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	unsigned int *word = ram->get_word(addr);
	*word = get_control_register(instr.rt);
}

// MTCz rt, rd
void Coprocessor0::move_to_cop(const register_instruction& instr) 
{
	unsigned int value = cpu->get_register(instr.rt);
	set_control_register(instr.rs, value);
}

// MFCz rt, rd
void Coprocessor0::move_from_cop(const register_instruction& instr) 
{
	unsigned int value = get_control_register(instr.rd);
	cpu->set_register(instr.rt, value);
}

// CTCz rt, rd
void Coprocessor0::move_control_to_cop(const register_instruction& instr) 
{
	move_to_cop(instr);
}

// CFCz rt, rd
void Coprocessor0::move_control_from_cop(const register_instruction& instr) 
{
	move_from_cop(instr);
}

// COPz cofun
void Coprocessor0::move_control_to_cop_fun(const register_instruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::move_to_cp0(const register_instruction& instr)
{
	move_to_cop(instr);
}

void Coprocessor0::move_from_cp0(const register_instruction& instr)
{
	move_from_cop(instr);
}

void Coprocessor0::read_indexed_tlb_entry(const register_instruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::write_indexed_tlb_entry(const register_instruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::write_random_tlb_entry(const register_instruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::probe_tlb_for_matching_entry(const register_instruction& instr)
{
	throw std::logic_error("not supported on cop0");
}

void Coprocessor0::restore_from_exception(const register_instruction& instr)
{
	throw std::logic_error("not supported on cop0");
}