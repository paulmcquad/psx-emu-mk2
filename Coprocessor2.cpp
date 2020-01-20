#include "Coprocessor2.hpp"
#include "InstructionTypes.hpp"
#include "Cpu.hpp"
#include "Ram.hpp"
#include <stdexcept>

enum class gte_commands : unsigned char
{
	RTPS   = 0x01,
	NCLIP  = 0x06,
	OP_sf  = 0x0c,
	DPCS   = 0x10,
	INTPL  = 0x11,
	MVMVA  = 0x12,
	NCDS   = 0x13,
	CDP    = 0x14,
	NCDT   = 0x16,
	NCCS   = 0x1b,
	NCS    = 0x1e,
	NCT    = 0x20,
	SQR_sf = 0x28,
	DCPL   = 0x29,
	DPCT   = 0x2a,
	AVSZ3  = 0x2d,
	AVSZ4  = 0x2e,
	RTPT   = 0x30,
	GPF_sf = 0x3d,
	GPL_sf = 0x3e,
	NCCT   = 0x3f
};

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

void Coprocessor2::load_word_to_cop(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	unsigned int *word = ram->get_word(addr);
	set_data_register(instr.rt, *word);
}

void Coprocessor2::store_word_from_cop(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)cpu->get_register(instr.rs);
	*ram->get_word(addr) = get_data_register(instr.rt);
}

void Coprocessor2::move_to_cop(const RegisterInstruction& instr)
{
	unsigned int value = cpu->get_register(instr.rt);
	set_data_register(instr.rd, value);
}

void Coprocessor2::move_from_cop(const RegisterInstruction& instr)
{
	unsigned value = get_data_register(instr.rd);
	cpu->set_register(instr.rs, value);
}

void Coprocessor2::move_control_to_cop(const RegisterInstruction& instr)
{
	unsigned int value = cpu->get_register(instr.rt);
	set_control_register(instr.rd, value);
}

void Coprocessor2::move_control_from_cop(const RegisterInstruction& instr)
{
	unsigned int value = get_control_register(instr.rd);
	cpu->set_register(instr.rt, value);
}

void Coprocessor2::move_control_to_cop_fun(const RegisterInstruction& instr)
{

}