#pragma once
#include <memory>
#include "InstructionTypes.hpp"
#include <sstream>

class Cop
{
public:

	virtual void save_state(std::stringstream& file) = 0;
	virtual void load_state(std::stringstream& file) = 0;

	virtual void execute(const instruction_union& instruction) = 0;

	virtual void load_word_to_cop(const instruction_union& instr) = 0;
	virtual void store_word_from_cop(const instruction_union& instr) = 0;
	virtual void move_to_cop(const instruction_union& instr) = 0;
	virtual void move_from_cop(const instruction_union& instr) = 0;
	virtual void move_control_to_cop(const instruction_union& instr) = 0;
	virtual void move_control_from_cop(const instruction_union& instr) = 0;
	virtual void move_control_to_cop_fun(const instruction_union& instr) = 0;

	virtual void reset() {};
};