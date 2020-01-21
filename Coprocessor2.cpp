#include "Coprocessor2.hpp"
#include "InstructionTypes.hpp"
#include "InstructionEnums.hpp"
#include "Cpu.hpp"
#include "Ram.hpp"
#include <stdexcept>

Coprocessor2::Coprocessor2(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Coprocessor(_ram, _cpu)
{

}

void Coprocessor2::execute(unsigned int instruction)
{
	immediate_instruction imm_instr(instruction);
	switch (static_cast<cpu_instructions>(imm_instr.op))
	{
		case cpu_instructions::LWC2:
		{
			load_word_to_cop(imm_instr);
		} return;

		case cpu_instructions::SWC2:
		{
			store_word_from_cop(imm_instr);
		} return;
	}

	register_instruction reg_instr(instruction);

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

unsigned int Coprocessor2::get_data_register(unsigned int index)
{
	return data_registers[index];
}

void Coprocessor2::set_data_register(unsigned int index, unsigned int value)
{
	data_registers[index] = value;
}

unsigned int Coprocessor2::get_control_register(unsigned int index)
{
	return control_registers[index];
}

void Coprocessor2::set_control_register(unsigned int index, unsigned int value)
{
	control_registers[index] = value;
}

void Coprocessor2::load_word_to_cop(const immediate_instruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	unsigned int *word = ram->get_word(addr);
	set_data_register(instr.rt, *word);
}

void Coprocessor2::store_word_from_cop(const immediate_instruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	*ram->get_word(addr) = get_data_register(instr.rt);
}

void Coprocessor2::move_to_cop(const register_instruction& instr)
{
	unsigned int value = cpu->get_register(instr.rt);
	set_data_register(instr.rd, value);
}

void Coprocessor2::move_from_cop(const register_instruction& instr)
{
	unsigned value = get_data_register(instr.rd);
	cpu->set_register(instr.rs, value);
}

void Coprocessor2::move_control_to_cop(const register_instruction& instr)
{
	unsigned int value = cpu->get_register(instr.rt);
	set_control_register(instr.rd, value);
}

void Coprocessor2::move_control_from_cop(const register_instruction& instr)
{
	unsigned int value = get_control_register(instr.rd);
	cpu->set_register(instr.rt, value);
}

void Coprocessor2::move_control_to_cop_fun(const register_instruction& instr)
{
	throw std::logic_error("not implemented");
}