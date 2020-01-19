#include "Coprocessor2.hpp"

Coprocessor2::Coprocessor2(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Coprocessor(_ram, _cpu)
{

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