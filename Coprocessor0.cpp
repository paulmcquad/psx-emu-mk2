#include "Coprocessor0.hpp"

Coprocessor0::Coprocessor0(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Coprocessor(_ram, _cpu)
{
	
}

void Coprocessor0::load_word_to_cop(const ImmediateInstruction& instr) 
{

}

void Coprocessor0::store_word_from_cop(const ImmediateInstruction& instr) 
{

}

void Coprocessor0::move_to_cop(const RegisterInstruction& instr) 
{

}

void Coprocessor0::move_from_cop(const RegisterInstruction& instr) 
{

}

void Coprocessor0::move_control_to_cop(const RegisterInstruction& instr) 
{

}

void Coprocessor0::move_control_from_cop(const RegisterInstruction& instr) 
{

}

void Coprocessor0::move_control_to_cop_fun(const RegisterInstruction& instr)
{

}