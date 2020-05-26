#pragma once
#include <string>
#include "../InstructionTypes.hpp"

class MipsToString
{
public:
	static std::string to_string(instruction_union instruction)
	{
		if (instruction.raw == 0x0)
		{
			return "NOP";
		}

		cpu_instructions opcode = static_cast<cpu_instructions>(instruction.jump_instruction.op);

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
		case cpu_instructions::LWC0:
			opcode_str = "LWC0";
			break;
		case cpu_instructions::LWC2:
			opcode_str = "LWC2";
			break;
		case cpu_instructions::SWC0:
			opcode_str = "SWC0";
			break;
		case cpu_instructions::SWC2:
			opcode_str = "SWC2";
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
			cpu_bconds cond = static_cast<cpu_bconds>(instruction.immediate_instruction.rt);
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
			cpu_special_funcs special_opcode = static_cast<cpu_special_funcs>(instruction.register_instruction.funct);
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
			case cpu_special_funcs::MTHI:
				opcode_str = "MTHI";
				break;
			case cpu_special_funcs::MTLO:
				opcode_str = "MTLO";
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
			int offset = (short)instruction.immediate_instruction.immediate;
			param_str_stream << "rt[" << instruction.immediate_instruction.rt << "], @rs[" << instruction.immediate_instruction.rs << "]+0x" << std::hex << offset;
		} break;

		case cpu_instructions::ADDI:
		case cpu_instructions::ADDIU:
		case cpu_instructions::SLTI:
		case cpu_instructions::SLTIU:
		case cpu_instructions::ANDI:
		case cpu_instructions::ORI:
		case cpu_instructions::XORI:
		{
			int immediate = (short)instruction.immediate_instruction.immediate;
			param_str_stream << "rt[" << instruction.immediate_instruction.rt << "], rs[" << instruction.immediate_instruction.rs << "], 0x" << std::hex << immediate;
		} break;

		case cpu_instructions::LUI:
		{
			int immediate = instruction.immediate_instruction.immediate << 16;
			param_str_stream << "rt[" << instruction.immediate_instruction.rt << "], 0x" << std::hex << immediate;
		} break;

		case cpu_instructions::J:
		case cpu_instructions::JAL:
		{
			unsigned int target = instruction.jump_instruction.target << 2;
			param_str_stream << "@0x" << std::hex << target;
		} break;


		case cpu_instructions::BLEZ:
		case cpu_instructions::BGTZ:
		case cpu_instructions::BCOND:
		{
			int offset = (short)instruction.immediate_instruction.immediate << 2;
			param_str_stream << "rs[" << instruction.immediate_instruction.rs << "], @+0x" << std::hex << offset;
		} break;

		case cpu_instructions::BEQ:
		case cpu_instructions::BNE:
		{
			int offset = (short)instruction.immediate_instruction.immediate << 2;
			param_str_stream << "rs[" << instruction.immediate_instruction.rs << "], rt[" << instruction.immediate_instruction.rt << "], @+0x" << std::hex << offset;
		} break;

		case cpu_instructions::SPECIAL:
		{
			cpu_special_funcs special_opcode = static_cast<cpu_special_funcs>(instruction.register_instruction.funct);
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
				param_str_stream << "rd[" << instruction.register_instruction.rd << "], rs[" << instruction.register_instruction.rs << "], rt[" << instruction.register_instruction.rt << "]";
			} break;

			case cpu_special_funcs::SLL:
			case cpu_special_funcs::SRL:
			case cpu_special_funcs::SRA:
			{
				param_str_stream << "rd[" << instruction.register_instruction.rd << "], rt[" << instruction.register_instruction.rt << "], " << instruction.register_instruction.shamt;
			} break;

			case cpu_special_funcs::SLLV:
			case cpu_special_funcs::SRLV:
			case cpu_special_funcs::SRAV:
			{
				param_str_stream << "rd[" << instruction.register_instruction.rd << "], rt[" << instruction.register_instruction.rt << "], rs[" << instruction.register_instruction.rs << "]";
			} break;

			case cpu_special_funcs::MULT:
			case cpu_special_funcs::MULTU:
			case cpu_special_funcs::DIV:
			case cpu_special_funcs::DIVU:
			{
				param_str_stream << "rs[" << instruction.register_instruction.rs << "], rt[" << instruction.register_instruction.rt << "]";
			} break;

			case cpu_special_funcs::MFHI:
			case cpu_special_funcs::MFLO:
			case cpu_special_funcs::MTHI:
			case cpu_special_funcs::MTLO:
			{
				param_str_stream << "rd[" << instruction.register_instruction.rd << "]";
			} break;

			case cpu_special_funcs::JR:
			{
				param_str_stream << "@rs[" << instruction.register_instruction.rs << "]";
			} break;

			case cpu_special_funcs::JALR:
			{
				param_str_stream << "@rs[" << instruction.register_instruction.rs << "], rd[" << instruction.register_instruction.rd << "]";
			} break;

			default:
				param_str_stream << "";
			};
		} break;

		// I only include 0 and 2, as the psx doesn't use any other co-processors
		case cpu_instructions::COP0:
		case cpu_instructions::COP2:
		case cpu_instructions::LWC0:
		case cpu_instructions::LWC2:
		case cpu_instructions::SWC0:
		case cpu_instructions::SWC2:
		{
			if (opcode == cpu_instructions::LWC0 || opcode == cpu_instructions::LWC2 ||
				opcode == cpu_instructions::SWC0 || opcode == cpu_instructions::SWC2)
			{
				int offset = (short)instruction.immediate_instruction.immediate;
				param_str_stream << "rt[" << instruction.immediate_instruction.rt << "], @rs[" << instruction.immediate_instruction.rs << "]+0x" << std::hex << offset;
			}
			else if (opcode == cpu_instructions::COP0 && instruction.register_instruction.rs == 0b10000)
			{
				param_str_stream << "RFE";
			}
			else
			{
				copz_instructions function = static_cast<copz_instructions>(instruction.register_instruction.rs);
				switch (function) {
					case copz_instructions::MT:
					case copz_instructions::MF:
					case copz_instructions::CF:
					case copz_instructions::CT:
					{
						param_str_stream << "rt[" << instruction.register_instruction.rt << "], rd[" << instruction.register_instruction.rd << "]";
					} break;

					// it can only be the coprocessor function
					default:
					{
						param_str_stream << "cofun: " << std::hex << (instruction.raw && 0x1FFFFFF);
					}
				}
			}
		} break;

		default:
			param_str_stream << "";
		}

		return opcode_str + " " + param_str_stream.str();
	}
};