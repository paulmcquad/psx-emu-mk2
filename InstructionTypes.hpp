#pragma once
#include "InstructionEnums.hpp"
#include <string>
#include <sstream>

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

	std::string to_string()
	{
		if (raw == 0x0)
		{
			return "NOP";
		}

		cpu_instructions opcode = static_cast<cpu_instructions>(jump_instruction.op);

		std::string opcode_str = "";

		// get opcode
		switch (opcode)
		{
		case cpu_instructions::ADDI:
			opcode_str = "ADDI";
			break;
		case cpu_instructions::ADDIU:
			opcode_str = "ADDIU";
			break;
		case cpu_instructions::ANDI:
			opcode_str = "ANDI";
			break;
		case cpu_instructions::BEQ:
			opcode_str = "BEQ";
			break;
		case cpu_instructions::BGTZ:
			opcode_str = "BGTZ";
			break;
		case cpu_instructions::BLEZ:
			opcode_str = "BLEZ";
			break;
		case cpu_instructions::BNE:
			opcode_str = "BNE";
			break;
		case cpu_instructions::COP0:
			opcode_str = "COP0";
			break;
		case cpu_instructions::COP2:
			opcode_str = "COP2";
			break;
		case cpu_instructions::J:
			opcode_str = "J";
			break;
		case cpu_instructions::JAL:
			opcode_str = "JAL";
			break;
		case cpu_instructions::LB:
			opcode_str = "LB";
			break;
		case cpu_instructions::LBU:
			opcode_str = "LBU";
			break;
		case cpu_instructions::LH:
			opcode_str = "LH";
			break;
		case cpu_instructions::LHU:
			opcode_str = "LHU";
			break;
		case cpu_instructions::LUI:
			opcode_str = "LUI";
			break;
		case cpu_instructions::LW:
			opcode_str = "LW";
			break;
		case cpu_instructions::LWC0:
			opcode_str = "LWC0";
			break;
		case cpu_instructions::LWC2:
			opcode_str = "LWC2";
			break;
		case cpu_instructions::LWL:
			opcode_str = "LWL";
			break;
		case cpu_instructions::LWR:
			opcode_str = "LWR";
			break;
		case cpu_instructions::ORI:
			opcode_str = "ORI";
			break;
		case cpu_instructions::SB:
			opcode_str = "SB";
			break;
		case cpu_instructions::SH:
			opcode_str = "SH";
			break;
		case cpu_instructions::SLTI:
			opcode_str = "SLTI";
			break;
		case cpu_instructions::SLTIU:
			opcode_str = "SLTIU";
			break;
		case cpu_instructions::SW:
			opcode_str = "SW";
			break;
		case cpu_instructions::SWC0:
			opcode_str = "SWC0";
			break;
		case cpu_instructions::SWC2:
			opcode_str = "SWC2";
			break;
		case cpu_instructions::SWL:
			opcode_str = "SWL";
			break;
		case cpu_instructions::SWR:
			opcode_str = "SWR";
			break;
		case cpu_instructions::XORI:
			opcode_str = "XORI";
			break;
		default:
			opcode_str = "UNKNOWN";
		}

		// get bcond opcode
		if (opcode == cpu_instructions::BCOND)
		{
			cpu_bconds cond = static_cast<cpu_bconds>(immediate_instruction.rt);
			switch (cond)
			{
			case cpu_bconds::BGEZ:
				opcode_str = "BGEZ";
				break;
			case cpu_bconds::BGEZAL:
				opcode_str = "BGEZAL";
				break;
			case cpu_bconds::BLTZ:
				opcode_str = "BLTZ";
				break;
			case cpu_bconds::BLTZAL:
				opcode_str = "BLTZAL";
				break;
			default:
				opcode_str = "UNKNOWN";
			};
		}

		// get special opcode
		if (opcode == cpu_instructions::SPECIAL)
		{
			cpu_special_funcs special_opcode = static_cast<cpu_special_funcs>(register_instruction.funct);
			switch (special_opcode)
			{
			case cpu_special_funcs::ADD:
				opcode_str = "ADD";
				break;
			case cpu_special_funcs::ADDU:
				opcode_str = "ADDU";
				break;
			case cpu_special_funcs::AND:
				opcode_str = "AND";
				break;
			case cpu_special_funcs::BREAK:
				opcode_str = "BREAK";
				break;
			case cpu_special_funcs::DIV:
				opcode_str = "DIV";
				break;
			case cpu_special_funcs::DIVU:
				opcode_str = "DIVU";
				break;
			case cpu_special_funcs::JR:
				opcode_str = "JR";
				break;
			case cpu_special_funcs::JALR:
				opcode_str = "JALR";
				break;
			case cpu_special_funcs::MFHI:
				opcode_str = "MFHI";
				break;
			case cpu_special_funcs::MFLO:
				opcode_str = "MFLO";
				break;
			case cpu_special_funcs::MULT:
				opcode_str = "MULT";
				break;
			case cpu_special_funcs::MULTU:
				opcode_str = "MULTU";
				break;
			case cpu_special_funcs::NOR:
				opcode_str = "NOR";
				break;
			case cpu_special_funcs::OR:
				opcode_str = "OR";
				break;
			case cpu_special_funcs::SLL:
				opcode_str = "SLL";
				break;
			case cpu_special_funcs::SLLV:
				opcode_str = "SLLV";
				break;
			case cpu_special_funcs::SLT:
				opcode_str = "SLT";
				break;
			case cpu_special_funcs::SLTU:
				opcode_str = "SLTU";
				break;
			case cpu_special_funcs::SRA:
				opcode_str = "SRA";
				break;
			case cpu_special_funcs::SRAV:
				opcode_str = "SRAV";
				break;
			case cpu_special_funcs::SRL:
				opcode_str = "SRL";
				break;
			case cpu_special_funcs::SRLV:
				opcode_str = "SRLV";
				break;
			case cpu_special_funcs::SUB:
				opcode_str = "SUB";
				break;
			case cpu_special_funcs::SUBU:
				opcode_str = "SUBU";
				break;
			case cpu_special_funcs::SYSCALL:
				opcode_str = "SYSCALL";
				break;
			case cpu_special_funcs::XOR:
				opcode_str = "XOR";
				break;
			default:
				opcode_str = "UNKNOWN";
			}
		}

		// get params
		std::stringstream param_str_stream;
		// determine type of instruction
		switch (opcode)
		{
			// load/store
			case cpu_instructions::LB:
			case cpu_instructions::LBU:
			case cpu_instructions::LH:
			case cpu_instructions::LHU:
			case cpu_instructions::LW:
			case cpu_instructions::LWL:
			case cpu_instructions::LWR:
			case cpu_instructions::SB:
			case cpu_instructions::SH:
			case cpu_instructions::SW:
			case cpu_instructions::SWL:
			case cpu_instructions::SWR:
			{
				int offset = (short)immediate_instruction.immediate;
				param_str_stream << "rt[" << immediate_instruction.rt << "], @rs[" << immediate_instruction.rs << "]+0x" << std::hex << offset;
			} break;

			case cpu_instructions::ADDI:
			case cpu_instructions::ADDIU:
			case cpu_instructions::SLTI:
			case cpu_instructions::SLTIU:
			case cpu_instructions::ANDI:
			case cpu_instructions::ORI:
			case cpu_instructions::XORI:
			{
				int immediate = (short)immediate_instruction.immediate;
				param_str_stream << "rt[" << immediate_instruction.rt << "], rs[" << immediate_instruction.rs << "], 0x" << std::hex << immediate;
			} break;

			case cpu_instructions::LUI:
			{
				int immediate = immediate_instruction.immediate << 16;
				param_str_stream << "rt[" << immediate_instruction.rt << "], 0x" << std::hex << immediate;
			} break;

			case cpu_instructions::J:
			case cpu_instructions::JAL:
			{
				unsigned int target = jump_instruction.target << 2;
				param_str_stream << "@0x" << std::hex << target;
			} break;


			case cpu_instructions::BLEZ:
			case cpu_instructions::BGTZ:
			case cpu_instructions::BCOND:
			{
				int offset = (short)immediate_instruction.immediate << 2;
				param_str_stream << "rs[" << immediate_instruction.rs << "], @+0x" << std::hex << offset;
			} break;

			case cpu_instructions::BEQ:
			case cpu_instructions::BNE:
			{
				int offset = (short)immediate_instruction.immediate << 2;
				param_str_stream << "rs[" << immediate_instruction.rs << "], rt[" << immediate_instruction.rt << "], @+0x" << std::hex << offset;
			} break;

			case cpu_instructions::SPECIAL:
			{
				cpu_special_funcs special_opcode = static_cast<cpu_special_funcs>(register_instruction.funct);
				switch (special_opcode)
				{
					case cpu_special_funcs::ADD:
					case cpu_special_funcs::ADDU:
					case cpu_special_funcs::SUB:
					case cpu_special_funcs::SUBU:
					case cpu_special_funcs::SLT:
					case cpu_special_funcs::SLTU:
					case cpu_special_funcs::AND:
					case cpu_special_funcs::OR:
					case cpu_special_funcs::XOR:
					case cpu_special_funcs::NOR:
					{
						param_str_stream << "rd[" << register_instruction.rd << "], rs[" << register_instruction.rs << "], rt[" << register_instruction.rt << "]";
					} break;

					case cpu_special_funcs::SLL:
					case cpu_special_funcs::SRL:
					case cpu_special_funcs::SRA:
					{
						param_str_stream << "rd[" << register_instruction.rd << "], rt[" << register_instruction.rt << "], " << register_instruction.shamt;
					} break;

					case cpu_special_funcs::SLLV:
					case cpu_special_funcs::SRLV:
					case cpu_special_funcs::SRAV:
					{
						param_str_stream << "rd[" << register_instruction.rd << "], rt[" << register_instruction.rt << "], rs[" << register_instruction.rs << "]";
					} break;

					case cpu_special_funcs::MULT:
					case cpu_special_funcs::MULTU:
					case cpu_special_funcs::DIV:
					case cpu_special_funcs::DIVU:
					{
						param_str_stream << "rs[" << register_instruction.rs << "], rt[" << register_instruction.rt << "]";
					} break;

					case cpu_special_funcs::MFHI:
					case cpu_special_funcs::MFLO:
					case cpu_special_funcs::MTHI:
					case cpu_special_funcs::MTLO:
					{
						param_str_stream << "rd[" << register_instruction.rd << "]";
					} break;

					case cpu_special_funcs::JR:
					{
						param_str_stream << "@rs[" << register_instruction.rs << "]";
					} break;

					case cpu_special_funcs::JALR:
					{
						param_str_stream << "@rs[" << register_instruction.rs << "], rd[" << register_instruction.rd << "]";
					} break;

					default:
						param_str_stream << "";
				};
			} break;

			default:
				param_str_stream << "";
		}

		return opcode_str + " " + param_str_stream.str();
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
		unsigned short data0;
		unsigned short data1;
	} pixel_data;

	struct
	{
		unsigned int tex_x : 8;
		unsigned int tex_y : 8;
		unsigned int clut_x : 6;
		unsigned int clut_y : 9;
		unsigned int na : 1;
	} tex_palette;

	struct
	{
		unsigned int tex_x : 8;
		unsigned int tex_y : 8;
		unsigned int tex_page_base_x : 4;
		unsigned int tex_page_base_y : 1;
		unsigned int semi_transparency : 2;
		unsigned int tex_page_colors : 2;
		unsigned int na1 : 2;
		unsigned int tex_disable : 1;
		unsigned int na2 : 4;
	} tex_page;

	struct
	{
		unsigned int x_pos : 10;
		unsigned int na0 : 6;
		unsigned int y_pos : 9;
		unsigned int na1 : 7;
	} dest_coord;

	struct
	{
		unsigned int x_coord : 10;
		unsigned int y_coord : 9;
		unsigned int na1 : 5;
		// only on newer GPUs
		unsigned int y_coord2 : 10;
		unsigned int na2 : 4;
		unsigned int op : 8;
	} draw_area;

	struct
	{
		unsigned int x_siz : 10;
		unsigned int na0 : 6;
		unsigned int y_siz : 9;
		unsigned int na1 : 7;
	} dims;

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

	struct
	{
		unsigned int horizontal_res : 2;
		unsigned int vertical_res : 1;
		unsigned int video_mode : 1;
		unsigned int display_color_depth : 1;
		unsigned int vertical_interlace : 1;
		unsigned int horizontal_res_2 : 1;
		unsigned int reverse_flag : 1;
		unsigned int na : 24;
	} display_mode;

	gp_command(unsigned int val)
	{
		raw = val;
	}

	gp_command()
	{
		raw = 0x0;
	}
};