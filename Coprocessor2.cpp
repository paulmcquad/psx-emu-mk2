#include "Coprocessor2.hpp"
#include "InstructionTypes.hpp"
#include "InstructionEnums.hpp"
#include "Cpu.hpp"
#include "MemoryMap.hpp"
#include <stdexcept>

Cop2::Cop2(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Cop(_ram, _cpu)
{

}

void Cop2::execute(const instruction_union& instruction)
{
	switch (static_cast<cpu_instructions>(instruction.immediate_instruction.op))
	{
		case cpu_instructions::LWC2:
		{
			load_word_to_cop(instruction);
		} return;

		case cpu_instructions::SWC2:
		{
			store_word_from_cop(instruction);
		} return;
	}

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

unsigned int Cop2::get_data_register(unsigned int index)
{
	return data_registers[index];
}

void Cop2::set_data_register(unsigned int index, unsigned int value)
{
	data_registers[index] = value;
}

unsigned int Cop2::get_control_register(unsigned int index)
{
	return control_registers[index];
}

void Cop2::set_control_register(unsigned int index, unsigned int value)
{
	control_registers[index] = value;
}

void Cop2::load_word_to_cop(const instruction_union& instr)
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)cpu->get_register(instr.immediate_instruction.rs);
	unsigned int word = ram->load_word(addr);
	set_data_register(instr.immediate_instruction.rt, word);
}

void Cop2::store_word_from_cop(const instruction_union& instr)
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)cpu->get_register(instr.immediate_instruction.rs);
	ram->store_word(addr,get_data_register(instr.immediate_instruction.rt));
}

void Cop2::move_to_cop(const instruction_union& instr)
{
	unsigned int value = cpu->get_register(instr.register_instruction.rt);
	set_data_register(instr.register_instruction.rd, value);
}

void Cop2::move_from_cop(const instruction_union& instr)
{
	unsigned value = get_data_register(instr.register_instruction.rd);
	cpu->set_register(instr.register_instruction.rs, value);
}

void Cop2::move_control_to_cop(const instruction_union& instr)
{
	unsigned int value = cpu->get_register(instr.register_instruction.rt);
	set_control_register(instr.register_instruction.rd, value);
}

void Cop2::move_control_from_cop(const instruction_union& instr)
{
	unsigned int value = get_control_register(instr.register_instruction.rd);
	cpu->set_register(instr.register_instruction.rt, value);
}

void Cop2::move_control_to_cop_fun(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}