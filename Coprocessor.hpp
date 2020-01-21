#pragma once
#include <memory>

struct immediate_instruction;
struct register_instruction;

class Ram;
class Cpu;

class Coprocessor
{
public:
	Coprocessor(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) { 
		ram = _ram;
		cpu = _cpu;
	};

	virtual void execute(unsigned int instruction) = 0;

	virtual void load_word_to_cop(const immediate_instruction& instr) = 0;
	virtual void store_word_from_cop(const immediate_instruction& instr) = 0;
	virtual void move_to_cop(const register_instruction& instr) = 0;
	virtual void move_from_cop(const register_instruction& instr) = 0;
	virtual void move_control_to_cop(const register_instruction& instr) = 0;
	virtual void move_control_from_cop(const register_instruction& instr) = 0;
	virtual void move_control_to_cop_fun(const register_instruction& instr) = 0;

	std::shared_ptr<Ram> ram = nullptr;
	std::shared_ptr<Cpu> cpu = nullptr;
};