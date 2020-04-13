#include <stdexcept>
#include <fstream>
#include "SystemControlCoprocessor.hpp"
#include "InstructionTypes.hpp"
#include "InstructionEnums.hpp"
#include "Ram.hpp"
#include "Cpu.hpp"
#include "Exceptions.hpp"

SystemControlCoprocessor::SystemControlCoprocessor(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu) :
	Cop(_ram, _cpu)
{
}

void SystemControlCoprocessor::save_state(std::ofstream& file)
{
	file.write(reinterpret_cast<char*>(&control_registers[0]), sizeof(unsigned int) * 32);
}

void SystemControlCoprocessor::load_state(std::ifstream& file)
{
	file.read(reinterpret_cast<char*>(&control_registers[0]), sizeof(unsigned int) * 32);
}

void SystemControlCoprocessor::execute(const instruction_union& instruction)
{
	switch (static_cast<cpu_instructions>(instruction.immediate_instruction.op))
	{
		// LWC0
		case cpu_instructions::LWC0:
		{
			load_word_to_cop(instruction);
		} return;

		case cpu_instructions::SWC0:
		{
			store_word_from_cop(instruction);
		} return;
	}
	
	if (instruction.register_instruction.rs == 0b10000)
	{
		cop0_instructions func = static_cast<cop0_instructions>(instruction.register_instruction.funct);
		switch (func) {
			case cop0_instructions::RFE:
			{
				restore_from_exception(instruction);
			} break;
			default:
				throw std::logic_error("not supported on cop0");
		}
	}
	else
	{
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
}

unsigned int SystemControlCoprocessor::get_control_register(SystemControlCoprocessor::register_names register_name)
{
	unsigned int index = static_cast<unsigned int>(register_name);
	return control_registers[index];
}

void SystemControlCoprocessor::set_control_register(SystemControlCoprocessor::register_names register_name, unsigned int value)
{
	unsigned int index = static_cast<unsigned int>(register_name);
	control_registers[index] = value;
}

unsigned int SystemControlCoprocessor::get_control_register(unsigned int index)
{
	return control_registers[index];
}

void SystemControlCoprocessor::set_control_register(unsigned int index, unsigned int value)
{
	control_registers[index] = value;
}

// LWCz rt, offset(base)
void SystemControlCoprocessor::load_word_to_cop(const instruction_union& instr) 
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)cpu->register_file.get_register(instr.immediate_instruction.rs);
	unsigned int word = ram->load_word(addr);
	set_control_register(instr.immediate_instruction.rt, word);
}

// SWCz rt, offset(base)
void SystemControlCoprocessor::store_word_from_cop(const instruction_union& instr)
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)cpu->register_file.get_register(instr.immediate_instruction.rs);
	unsigned int control_value = get_control_register(instr.immediate_instruction.rt);
	ram->store_word(addr, control_value);
}

// MTCz rt, rd
void SystemControlCoprocessor::move_to_cop(const instruction_union& instr)
{
	unsigned int value = cpu->register_file.get_register(instr.register_instruction.rt);
	set_control_register(instr.register_instruction.rd, value);
}

// MFCz rt, rd
void SystemControlCoprocessor::move_from_cop(const instruction_union& instr)
{
	unsigned int value = get_control_register(instr.register_instruction.rd);
	cpu->register_file.set_register(instr.register_instruction.rt, value);
}

// CTCz rt, rd
void SystemControlCoprocessor::move_control_to_cop(const instruction_union& instr)
{
	move_to_cop(instr);
}

// CFCz rt, rd
void SystemControlCoprocessor::move_control_from_cop(const instruction_union& instr)
{
	move_from_cop(instr);
}

// COPz cofun
void SystemControlCoprocessor::move_control_to_cop_fun(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}

void SystemControlCoprocessor::move_to_cp0(const instruction_union& instr)
{
	move_to_cop(instr);
}

void SystemControlCoprocessor::move_from_cp0(const instruction_union& instr)
{
	move_from_cop(instr);
}

void SystemControlCoprocessor::restore_from_exception(const instruction_union& instr)
{
	SystemControlCoprocessor::status_register sr = get<SystemControlCoprocessor::status_register>();

	unsigned int mode = sr.raw & 0x3f;
	sr.raw &= ~0x3f;
	sr.raw |= mode >> 2;

	set<SystemControlCoprocessor::status_register>(sr);
}