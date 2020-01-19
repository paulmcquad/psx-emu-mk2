#pragma once
#include <memory>

struct ImmediateInstruction;
struct RegisterInstruction;

class Ram;
class Cpu;

class Coprocessor
{
public:
	Coprocessor(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) { 
		ram = _ram;
		cpu = _cpu;
	};

	virtual void load_word_to_cop(const ImmediateInstruction& instr) = 0;
	virtual void store_word_from_cop(const ImmediateInstruction& instr) = 0;
	virtual void move_to_cop(const RegisterInstruction& instr) = 0;
	virtual void move_from_cop(const RegisterInstruction& instr) = 0;
	virtual void move_control_to_cop(const RegisterInstruction& instr) = 0;
	virtual void move_control_from_cop(const RegisterInstruction& instr) = 0;
	virtual void move_control_to_cop_fun(const RegisterInstruction& instr) = 0;

	std::shared_ptr<Ram> ram = nullptr;
	std::shared_ptr<Cpu> cpu = nullptr;
};