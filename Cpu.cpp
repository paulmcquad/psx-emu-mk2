#include "Ram.hpp"
#include "Cpu.hpp"
#include "Coprocessor.hpp"
#include "Coprocessor0.hpp"
#include "Coprocessor2.hpp"
#include "InstructionTypes.hpp"
#include "InstructionEnums.hpp"

constexpr unsigned int BIOS_START = 0xbfc00000;

void Cpu::init(std::shared_ptr<Ram> _ram)
{
	pc = BIOS_START;
	ram = _ram;
	next_instruction = 0x0;
	coprocessors[0] = std::make_shared<Coprocessor0>(ram, shared_from_this());
	coprocessors[2] = std::make_shared<Coprocessor2>(ram, shared_from_this());
}

void Cpu::tick()
{
	unsigned int current_pc = pc;
	unsigned int current_instruction = next_instruction;

	next_instruction = ram->load_word(current_pc);

	pc = current_pc + 4;
	execute(current_instruction);
}

void Cpu::execute(unsigned int instruction)
{
	immediate_instruction imm_instr(instruction);
	jump_instruction jmp_instr(instruction);
	cpu_instructions opcode = static_cast<cpu_instructions>(instruction >> 26);

	switch (opcode)
	{
		case cpu_instructions::SPECIAL:
		{
			execute_special(instruction);
		} break;

		case cpu_instructions::BCOND:
		{
			execute_bcond(instruction);
		} break;

		case cpu_instructions::J:
		{
			jump(jmp_instr);
		} break;

		case cpu_instructions::JAL:
		{
			jump_and_link(jmp_instr);
		} break;

		case cpu_instructions::BEQ:
		{
			branch_on_equal(imm_instr);
		} break;

		case cpu_instructions::BNE:
		{
			branch_on_not_equal(imm_instr);
		} break;

		case cpu_instructions::BLEZ:
		{
			branch_on_less_than_or_equal_zero(imm_instr);
		} break;

		case cpu_instructions::BGTZ:
		{
			branch_on_greater_than_zero(imm_instr);
		} break;

		case cpu_instructions::ADDI:
		{
			add_immediate(imm_instr);
		} break;

		case cpu_instructions::ADDIU:
		{
			add_immediate_unsigned(imm_instr);
		} break;

		case cpu_instructions::SLTI:
		{
			set_on_less_than_immediate(imm_instr);
		} break;

		case cpu_instructions::SLTIU:
		{
			set_on_less_than_unsigned_immediate(imm_instr);
		} break;

		case cpu_instructions::ANDI:
		{
			and_immediate(imm_instr);
		} break;

		case cpu_instructions::ORI:
		{
			or_immediate(imm_instr);
		} break;

		case cpu_instructions::XORI:
		{
			xor_immediate(imm_instr);
		} break;

		case cpu_instructions::LUI:
		{
			load_upper_immediate(imm_instr);
		} break;

		case cpu_instructions::LB:
		{
			load_byte(imm_instr);
		} break;

		case cpu_instructions::LH:
		{
			load_halfword(imm_instr);
		} break;

		case cpu_instructions::LWL:
		{
			load_word_left(imm_instr);
		} break;

		case cpu_instructions::LW:
		{
			load_word(imm_instr);
		} break;

		case cpu_instructions::LBU:
		{
			load_byte_unsigned(imm_instr);
		} break;

		case cpu_instructions::LHU:
		{
			load_halfword_unsigned(imm_instr);
		} break;

		case cpu_instructions::LWR:
		{
			load_word_right(imm_instr);
		} break;

		case cpu_instructions::SB:
		{
			store_byte(imm_instr);
		} break;

		case cpu_instructions::SH:
		{
			store_halfword(imm_instr);
		} break;

		case cpu_instructions::SWL:
		{
			store_word_left(imm_instr);
		} break;

		case cpu_instructions::SW:
		{
			store_word(imm_instr);
		} break;

		case cpu_instructions::SWR:
		{
			store_word_right(imm_instr);
		} break;

		case cpu_instructions::COP0:
		case cpu_instructions::COP2:
		case cpu_instructions::LWC0:
		case cpu_instructions::LWC2:
		case cpu_instructions::SWC0:
		case cpu_instructions::SWC2:
		{
			execute_cop(instruction);
		} break;

		default:
			throw std::logic_error("unimplemented opcode");
	}
}

void Cpu::execute_special(unsigned int instruction)
{
	register_instruction reg_instr(instruction);
	cpu_special_funcs func = static_cast<cpu_special_funcs>(reg_instr.funct);

	switch (func) {
		case cpu_special_funcs::ADD:
		{
			add(reg_instr);
		} break;

		case cpu_special_funcs::ADDU:
		{
			add_unsigned(reg_instr);
		} break;

		case cpu_special_funcs::AND:
		{
			and(reg_instr);
		} break;

		case cpu_special_funcs::BREAK:
		{
			breakpoint();
		} break;

		case cpu_special_funcs::DIV:
		{
			div(reg_instr);
		} break;

		case cpu_special_funcs::DIVU:
		{
			div_unsigned(reg_instr);
		} break;

		case cpu_special_funcs::JALR:
		{
			jump_and_link_register(reg_instr);
		} break;

		case cpu_special_funcs::JR:
		{
			jump_register(reg_instr);
		} break;

		case cpu_special_funcs::MFHI:
		{
			move_from_hi(reg_instr);
		} break;

		case cpu_special_funcs::MFLO:
		{
			move_from_lo(reg_instr);
		} break;

		case cpu_special_funcs::MTHI:
		{
			move_to_hi(reg_instr);
		} break;

		case cpu_special_funcs::MTLO:
		{
			move_to_lo(reg_instr);
		} break;

		case cpu_special_funcs::MULT:
		{
			mult(reg_instr);
		} break;

		case cpu_special_funcs::MULTU:
		{
			mult_unsigned(reg_instr);
		} break;

		case cpu_special_funcs::NOR:
		{
			nor(reg_instr);
		} break;

		case cpu_special_funcs::OR:
		{
			or(reg_instr);
		} break;

		case cpu_special_funcs::SLL:
		{
			shift_left_logical_variable(reg_instr);
		} break;

		case cpu_special_funcs::SLLV:
		{
			shift_left_logical_variable(reg_instr);
		} break;

		case cpu_special_funcs::SLT:
		{
			set_on_less_than(reg_instr);
		} break;

		case cpu_special_funcs::SLTU:
		{
			set_on_less_than_unsigned(reg_instr);
		} break;

		case cpu_special_funcs::SRA:
		{
			shift_right_arithmetic(reg_instr);
		} break;

		case cpu_special_funcs::SRAV:
		{
			shift_right_arithmetic_variable(reg_instr);
		} break;

		case cpu_special_funcs::SRL:
		{
			shift_right_logical(reg_instr);
		} break;

		case cpu_special_funcs::SRLV:
		{
			shift_right_arithmetic_variable(reg_instr);
		} break;

		case cpu_special_funcs::SUB:
		{
			sub(reg_instr);
		} break;

		case cpu_special_funcs::SUBU:
		{
			sub_unsigned(reg_instr);
		} break;

		case cpu_special_funcs::SYSCALL:
		{
			system_call();
		} break;

		case cpu_special_funcs::XOR:
		{
			xor(reg_instr);
		} break;
	}
}

void Cpu::execute_bcond(unsigned int instruction)
{
	immediate_instruction imm_instr(instruction);
	cpu_bconds cond = static_cast<cpu_bconds>(imm_instr.rt);

	switch (cond)
	{
		case cpu_bconds::BLTZ:
		{
			branch_on_less_than_zero(imm_instr);
		} break;

		case cpu_bconds::BGEZ:
		{
			branch_on_less_than_zero(imm_instr);
		} break;

		case cpu_bconds::BLTZAL:
		{
			branch_on_less_than_zero_and_link(imm_instr);
		} break;

		case cpu_bconds::BGEZAL:
		{
			branch_on_greater_than_or_equal_zero_and_link(imm_instr);
		} break;
	}
}

void Cpu::execute_cop(unsigned int instruction) 
{
	unsigned int cop_number = (instruction >> 26) & 0x3;
	coprocessors[cop_number]->execute(instruction);
}

unsigned int Cpu::get_register(int index) 
{
	return gp_registers[index];
}

void Cpu::set_register(int index, unsigned int value) 
{
	if (index != 0)
	{
		gp_registers[index] = value;
	}
}

unsigned int Cpu::get_immediate_base_addr(const immediate_instruction& instr)
{
	return (short)instr.immediate + (int)get_register(instr.rs);
}

// load/store
// LB rt, offset(base)
void Cpu::load_byte(const immediate_instruction& instr) 
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned char value = ram->load_byte(addr);

	set_register(instr.rt, value);
}

// LBU rt, offset(base)
void Cpu::load_byte_unsigned(const immediate_instruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned char value = ram->load_byte(addr);

	set_register(instr.rt, value);
}

// LH rt, offset(base)
void Cpu::load_halfword(const immediate_instruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned short value = ram->load_halfword(addr);

	set_register(instr.rt, value);
}

// LHU rt, offset(base)
void Cpu::load_halfword_unsigned(const immediate_instruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned short value = ram->load_halfword(addr);

	set_register(instr.rt, value);
}

// LW rt, offset(base)
void Cpu::load_word(const immediate_instruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned int value = ram->load_word(addr);

	set_register(instr.rt, value);
}

// LWL rt, offset(base)
void Cpu::load_word_left(const immediate_instruction& instr)
{
	throw std::logic_error("not implemented");
}

// LWR rt, offset(base)
void Cpu::load_word_right(const immediate_instruction& instr)
{
	throw std::logic_error("not implemented");
}

// SB rt, offset(base)
void Cpu::store_byte(const immediate_instruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	ram->store_byte(addr, get_register(instr.rt));
}

// SH rt, offset(base)
void Cpu::store_halfword(const immediate_instruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	ram->store_halfword(addr, get_register(instr.rt));
}

// SW rt, offset(base)
void Cpu::store_word(const immediate_instruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	ram->store_word(addr, get_register(instr.rt));
}

// SWL rt, offset(base)
void Cpu::store_word_left(const immediate_instruction& instr)
{
	throw std::logic_error("not implemented");
}

// SWR rt, offset(base)
void Cpu::store_word_right(const immediate_instruction& instr)
{
	throw std::logic_error("not implemented");
}

// ALU immediate operations
// ADDI rt, rs, immediate
void Cpu::add_immediate(const immediate_instruction& instr)
{
	int immediate = instr.immediate;
	int rs_value = get_register(instr.rs);

	unsigned int value = (unsigned int)rs_value + (unsigned int)immediate;

	bool overflow = false;
	if (immediate > 0 && rs_value > 0) {
		if ((int)value < 0)
		{
			overflow = true;
		}
	}
	else if (immediate < 0 && rs_value < 0) {
		if ((int)value > 0)
		{
			overflow = true;
		}
	}

	if (overflow)
	{
		throw std::logic_error("overflow not implemented");
	}

	set_register(instr.rt, value);
}

// ADDIU rt, rs, immediate
void Cpu::add_immediate_unsigned(const immediate_instruction& instr)
{
	unsigned int value = get_register(instr.rs) + (int)instr.immediate;
	set_register(instr.rt, value);
}

// SLTI rt, rs, immediate
void Cpu::set_on_less_than_immediate(const immediate_instruction& instr)
{
	int rs_value = get_register(instr.rs);
	short immediate_value = instr.immediate;

	set_register(instr.rt, rs_value < immediate_value ? 1 : 0);
}

// SLTIU rt, rs, immediate
void Cpu::set_on_less_than_unsigned_immediate(const immediate_instruction& instr)
{
	int rs_value = get_register(instr.rs);
	short immediate_value = instr.immediate;

	set_register(instr.rt, rs_value < immediate_value ? 1 : 0);
}

// ANDI rt, rs, immediate
void Cpu::and_immediate(const immediate_instruction& instr)
{
	unsigned int value = (unsigned int)instr.immediate & get_register(instr.rs);
	set_register(instr.rt, value);
}

// ORI rt, rs, immediate
void Cpu::or_immediate(const immediate_instruction& instr)
{
	unsigned int value = (unsigned int)instr.immediate | gp_registers[instr.rs];
	set_register(instr.rt, value);
}

// XORI rt, rs, immediate
void Cpu::xor_immediate(const immediate_instruction& instr)
{
	unsigned int value = (unsigned int)instr.immediate ^ get_register(instr.rs);
	set_register(instr.rt, value);
}

// LUI rt, immediate
void Cpu::load_upper_immediate(const immediate_instruction& instr)
{
	unsigned int value = (unsigned int)(instr.immediate) << 16;
	set_register(instr.rt, value);
}

// three operand register type
// ADD rd, rs, rt
void Cpu::add(const register_instruction& instr)
{
	unsigned int value = gp_registers[instr.rs] + gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// ADDU rd, rs, rt
void Cpu::add_unsigned(const register_instruction& instr)
{
	unsigned int value = gp_registers[instr.rs] + gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// SUB rd, rs, rt
void Cpu::sub(const register_instruction& instr)
{
	unsigned int value = gp_registers[instr.rs] - gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// SUBU rd, rs, rt
void Cpu::sub_unsigned(const register_instruction& instr)
{
	unsigned int value = gp_registers[instr.rs] - gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// SLT rd, rs, rt
void Cpu::set_on_less_than(const register_instruction& instr)
{
	int rt_value = get_register(instr.rt);
	int rs_value = get_register(instr.rs);

	set_register(instr.rd, rs_value < rt_value ? 1 : 0);
}

// SLTU rd, rs, rt
void Cpu::set_on_less_than_unsigned(const register_instruction& instr)
{
	unsigned int rt_value = get_register(instr.rt);
	unsigned int rs_value = get_register(instr.rs);

	set_register(instr.rd, rs_value < rt_value ? 1 : 0);
}

// AND rd, rs, rt
void Cpu::and(const register_instruction& instr)
{
	unsigned int value = get_register(instr.rs) & get_register(instr.rt);
	set_register(instr.rd, value);
}

// OR rd, rs, rt
void Cpu::or(const register_instruction& instr)
{
	unsigned int value = get_register(instr.rs) | get_register(instr.rt);
	set_register(instr.rd, value);
}

// XOR rd, rs, rt
void Cpu::xor (const register_instruction& instr)
{
	unsigned int value = get_register(instr.rs) ^ get_register(instr.rt);
	set_register(instr.rd, value);
}

// NOR rd, rs, rt
void Cpu::nor(const register_instruction& instr)
{
	unsigned int value = ~(get_register(instr.rs) | get_register(instr.rt));
	set_register(instr.rd, value);
}

// shift operations
// SLL rd, rt, shamt
void Cpu::shift_left_logical(const register_instruction& instr)
{
	unsigned int value = get_register(instr.rt) << instr.shamt;
	set_register(instr.rd, value);
}

// SRL rd, rt, shamt
void Cpu::shift_right_logical(const register_instruction& instr)
{
	unsigned int value = get_register(instr.rt) >> instr.shamt;
	set_register(instr.rd, value);
}

// SRA rd, rt, shamt
void Cpu::shift_right_arithmetic(const register_instruction& instr)
{
	unsigned int value = (int)(get_register(instr.rt)) >> instr.shamt;
	set_register(instr.rd, value);
}

// SLLV rd, rt, rs
void Cpu::shift_left_logical_variable(const register_instruction& instr)
{
	unsigned int value = get_register(instr.rs) << (0x1F & get_register(instr.rt));
	set_register(instr.rd, value);
}

// SRLV rd, rt, rs
void Cpu::shift_right_logical_variable(const register_instruction& instr)
{
	unsigned int value = get_register(instr.rs) >> (0x1F & get_register(instr.rt));
	set_register(instr.rd, value);
}

// SRAV rd, rt, rs
void Cpu::shift_right_arithmetic_variable(const register_instruction& instr)
{
	unsigned int value = (int)get_register(instr.rs) >> (0x1F & get_register(instr.rt));
	set_register(instr.rd, value);
}

// multiply/divide
// MULT rs, rt
void Cpu::mult(const register_instruction& instr)
{
	long long result = (int)(get_register(instr.rs)) * (int)(get_register(instr.rt));
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// MULTU rs, rt
void Cpu::mult_unsigned(const register_instruction& instr)
{
	unsigned long long result = get_register(instr.rs) * get_register(instr.rt);
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// DIV rs, rt
void Cpu::div(const register_instruction& instr)
{
	hi = (int)get_register(instr.rs) / (int)get_register(instr.rt);
	lo = (int)get_register(instr.rs) % (int)get_register(instr.rt);
}

// DIVU rs, rt
void Cpu::div_unsigned(const register_instruction& instr)
{
	hi = get_register(instr.rs) / get_register(instr.rt);
	lo = get_register(instr.rs) % get_register(instr.rt);
}

// MFHI rd
void Cpu::move_from_hi(const register_instruction& instr)
{
	set_register(instr.rd, hi);
}

// MFLO rd
void Cpu::move_from_lo(const register_instruction& instr)
{
	set_register(instr.rd, lo);
}

// MTHI rd
void Cpu::move_to_hi(const register_instruction& instr)
{
	hi = get_register(instr.rs);
}

// MTLO rd
void Cpu::move_to_lo(const register_instruction& instr)
{
	lo = get_register(instr.rs);
}

// jump instructions
// J target
void Cpu::jump(const jump_instruction& instr)
{
	pc = (unsigned int)instr.target << 2 | (0xF0000000 & pc);
}

// JAL target
void Cpu::jump_and_link(const jump_instruction& instr)
{
	gp_registers[31] = pc + 8;
	pc = (unsigned int)instr.target << 2 | (0xF0000000 & pc);
}

// JR rs
void Cpu::jump_register(const register_instruction& instr)
{
	pc = get_register(instr.rs);
}

// JALR rs, rd
void Cpu::jump_and_link_register(const register_instruction& instr)
{
	gp_registers[31] = pc + 8;
	pc = get_register(instr.rs);
}

// branch instructions
// BEQ rs, rt, offset
void Cpu::branch_on_equal(const immediate_instruction& instr)
{
	if (get_register(instr.rt) == get_register(instr.rs)) {
		pc = (pc + 4) + ((int)instr.immediate << 2);
	}
}

// BNE rs, rt, offset
void Cpu::branch_on_not_equal(const immediate_instruction& instr)
{
	if (get_register(instr.rt) != get_register(instr.rs)) {
		unsigned int offset = (int)instr.immediate << 2;
		pc += offset - 4;
	}
}

// BLEZ rs, offset
void Cpu::branch_on_less_than_or_equal_zero(const immediate_instruction& instr)
{
	if (get_register(instr.rs) <= 0) {
		unsigned int offset = (int)instr.immediate << 2;
		pc += offset - 4;
	}
}

// BGTZ rs, offset
void Cpu::branch_on_greater_than_zero(const immediate_instruction& instr)
{
	if (get_register(instr.rs) > 0) {
		unsigned int offset = (int)instr.immediate << 2;
		pc += offset - 4;
	}
}

// BLTZ rs, offset
void Cpu::branch_on_less_than_zero(const immediate_instruction& instr)
{
	if (get_register(instr.rs) < 0) {
		unsigned int offset = (int)instr.immediate << 2;
		pc += offset - 4;
	}
}

// BGEZ rs, offset
void Cpu::branch_on_greater_than_or_equal_zero(const immediate_instruction& instr)
{
	if (get_register(instr.rs) <= 0) {
		unsigned int offset = (int)instr.immediate << 2;
		pc += offset - 4;
	}
}

// BLTZAL rs, offset
void Cpu::branch_on_less_than_zero_and_link(const immediate_instruction& instr)
{
	if (get_register(instr.rs) < 0) {
		throw std::logic_error("not implemented");
	}
}

// BGEZAL rs, offset
void Cpu::branch_on_greater_than_or_equal_zero_and_link(const immediate_instruction& instr)
{
	if (get_register(instr.rs) >= 0) {
		throw std::logic_error("not implemented");
	}
}

// special instructions
// SYSCALL
void Cpu::system_call()
{
	throw std::logic_error("not implemented");
}

// BREAK
void Cpu::breakpoint()
{
	throw std::logic_error("not implemented");
}

// co-processor instructions
// LWCz rt, offset(base)
void Cpu::load_word_to_cop(const immediate_instruction& instr)
{
	unsigned int cop_number = instr.op & 0x3;
	coprocessors[cop_number]->load_word_to_cop(instr);
}

// SWCz rt, offset(base)
void Cpu::store_word_from_cop(const immediate_instruction& instr)
{
	int cop_number = instr.op & 0x3;
	coprocessors[cop_number]->store_word_from_cop(instr);
}

// MTCz rt, rd
void Cpu::move_to_cop(const register_instruction& instr)
{
	int cop_number = instr.op & 0x3;
	coprocessors[cop_number]->move_to_cop(instr);
}

// MFCz rt, rd
void Cpu::move_from_cop(const register_instruction& instr)
{
	int cop_number = instr.op & 0x3;
	coprocessors[cop_number]->move_from_cop(instr);
}

// CTCz rt, rd
void Cpu::move_control_to_cop(const register_instruction& instr)
{
	int cop_number = instr.op & 0x3;
	coprocessors[cop_number]->move_control_to_cop(instr);
}

// CFCz rt, rd
void Cpu::move_control_from_cop(const register_instruction& instr)
{
	int cop_number = instr.op & 0x3;
	coprocessors[cop_number]->move_control_from_cop(instr);
}

// COPz cofun
void Cpu::move_control_to_cop_fun(const register_instruction& instr)
{
	unsigned int cop_number = instr.op & 0x3;
	coprocessors[cop_number]->move_control_to_cop_fun(instr);
}

// system control
// MTC0 rt, rd
void Cpu::move_to_cp0(const register_instruction& instr)
{
	throw std::logic_error("not implemented");
}

// MFC0 rt, rd
void Cpu::move_from_cp0(const register_instruction& instr)
{
	throw std::logic_error("not implemented");
}

// TLBR
void Cpu::read_indexed_tlb()
{
	throw std::logic_error("not implemented");
}

// TLBWI
void Cpu::write_indexed_tlb()
{
	throw std::logic_error("not implemented");
}

// TLBWR
void Cpu::write_random_tlb()
{
	throw std::logic_error("not implemented");
}

// TLBP
void Cpu::probe_tlb_for_matching_entry()
{
	throw std::logic_error("not implemented");
}

//RFE
void Cpu::restore_from_exception()
{
	throw std::logic_error("not implemented");
}