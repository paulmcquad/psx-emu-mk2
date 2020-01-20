#include "Coprocessor2.hpp"
#include "InstructionTypes.hpp"

Coprocessor2::Coprocessor2(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Coprocessor(_ram, _cpu)
{

}

void Coprocessor2::execute(unsigned int instruction)
{
	ImmediateInstruction imm_instr(instruction);
	switch (imm_instr.op)
	{
		// LWC2
		case 062:
		{
			load_word_to_cop(imm_instr);
		} return;

		// SWC2
		case 072:
		{
			store_word_from_cop(imm_instr);
		} return;
	}

	RegisterInstruction reg_instr(instruction);

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

void Coprocessor2::load_word_to_cop(const ImmediateInstruction& instr)
{

}

void Coprocessor2::store_word_from_cop(const ImmediateInstruction& instr)
{

}

void Coprocessor2::move_to_cop(const RegisterInstruction& instr)
{

}

void Coprocessor2::move_from_cop(const RegisterInstruction& instr)
{

}

void Coprocessor2::move_control_to_cop(const RegisterInstruction& instr)
{

}

void Coprocessor2::move_control_from_cop(const RegisterInstruction& instr)
{

}

void Coprocessor2::move_control_to_cop_fun(const RegisterInstruction& instr)
{

}