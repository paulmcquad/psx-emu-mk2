#pragma once
#include "InstructionEnums.hpp"

// using technically undefined behaviour but it's generally supported
union instruction_union
{
	instruction_union()
	{
		raw = 0x0;
	}
	instruction_union(cpu_instructions op, unsigned int rs, unsigned int rt, unsigned int immediate)
	{
		immediate_instruction.op = static_cast<unsigned int>(op);
		immediate_instruction.rs = rs;
		immediate_instruction.rt = rt;
		immediate_instruction.immediate = immediate;
	}

	instruction_union(cpu_instructions op, unsigned int target)
	{
		jump_instruction.op = static_cast<unsigned int>(op);
		jump_instruction.target = target;
	}

	instruction_union(cpu_instructions op, unsigned int rs, unsigned int rt, unsigned int rd, unsigned int shamt, cpu_special_funcs funct)
	{
		register_instruction.op = static_cast<unsigned int>(op);
		register_instruction.rs = rs;
		register_instruction.rt = rt;
		register_instruction.rd = rd;
		register_instruction.shamt = shamt;
		register_instruction.funct = static_cast<unsigned int>(funct);
	}

	instruction_union(unsigned int value)
	{
		raw = value;
	}

	unsigned int raw;
	struct
	{
		unsigned int immediate : 16;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int op : 6;
	} immediate_instruction;
	struct
	{
		unsigned int target : 26;
		unsigned int op : 6;
	} jump_instruction;
	struct
	{
		unsigned int funct : 6;
		unsigned int shamt : 5;
		unsigned int rd : 5;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int op : 6;
	} register_instruction;
};

union gp_command
{
	unsigned int raw;

	struct
	{
		signed short x;
		signed short y;
	} vert;

	struct
	{
		unsigned int x_pos : 10;
		unsigned int na0 : 6;
		unsigned int y_pos : 9;
		unsigned int na1 : 7;
	} dest_coord;

	struct
	{
		unsigned int x_siz : 10;
		unsigned int na0 : 6;
		unsigned int y_siz : 9;
		unsigned int na1 : 7;
	} width_height;

	struct
	{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char op;
	} color;

	struct
	{
		int x_offset : 11;
		int y_offset : 11;
		unsigned int na : 10;
	} draw_offset;

	gp_command(unsigned int val)
	{
		raw = val;
	}

	gp_command()
	{
		raw = 0x0;
	}
};