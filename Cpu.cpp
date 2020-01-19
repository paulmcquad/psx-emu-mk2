#include "Ram.hpp"
#include "Cpu.hpp"
#include "Coprocessor.hpp"
#include "Coprocessor0.hpp"
#include "Coprocessor2.hpp"
#include "InstructionTypes.hpp"

constexpr unsigned int BIOS_START = 0xbfc00000;

enum class opcodes : unsigned char
{
	SPECIAL = 000,
	BCOND   = 001,
	J       = 002,
	JAL     = 003,
	BEQ     = 004,
	BNE     = 005,
	BLEZ    = 006,
	BGTZ    = 007,
	ADDI    = 010,
	ADDIU   = 011,
	SLTI    = 012,
	SLTIU   = 013,
	ANDI    = 014,
	ORI     = 015,
	XORI    = 016,
	LUI     = 017,
	COP0    = 020,
	COP1    = 021,
	COP2    = 022,
	COP3    = 023,
	LB      = 040,
	LH      = 041,
	LWL     = 042,
	LW      = 043,
	LBU     = 044,
	LHU     = 045,
	LWR     = 046,
	SB      = 050,
	SH      = 051,
	SWL     = 052,
	SW      = 053,
	SWR     = 056,
	LWC0    = 060,
	LWC1    = 061,
	LWC2    = 062,
	LWC3    = 063,
	SWC0    = 070,
	SWC1    = 071,
	SWC2    = 072,
	SWC3    = 073
};

enum class special_funcs : unsigned char
{
	SLL     = 000,
	SRL     = 002,
	SRA     = 003,
	SLLV    = 004,
	SRLV    = 006,
	SRAV    = 007,
	JR      = 010,
	JALR    = 011,
	SYSCALL = 014,
	BREAK   = 015,
	MFHI    = 020,
	MTHI    = 021,
	MFLO    = 022,
	MTLO    = 023,
	MULT    = 030,
	MULTU   = 031,
	DIV     = 032,
	DIVU    = 033,
	ADD     = 040,
	ADDU    = 041,
	SUB     = 042,
	SUBU    = 043,
	AND     = 044,
	OR      = 045,
	XOR     = 046,
	NOR     = 047,
	SLT     = 052,
	SLTU    = 053
};

enum class bconds : unsigned char
{
	BLTZ   = 000,
	BGEZ   = 001,
	BLTZAL = 020,
	BGEZAL = 021
};

void Cpu::init(std::shared_ptr<Ram> _ram)
{
	pc = BIOS_START;
	ram = _ram;
	coprocessors[0] = std::make_shared<Coprocessor0>(ram, shared_from_this());
	coprocessors[2] = std::make_shared<Coprocessor2>(ram, shared_from_this());
}

void Cpu::reset()
{
	pc = BIOS_START;
}

void Cpu::run()
{
	execute(pc);
	pc += 4;
}

void Cpu::execute(unsigned int _pc)
{
	unsigned int instruction = *ram->get_word(_pc);
	ImmediateInstruction imm_instr(instruction);
	JumpInstruction jmp_instr(instruction);
	opcodes opcode = static_cast<opcodes>(instruction >> 26);

	switch (opcode)
	{
		case opcodes::SPECIAL:
		{
			execute_special(_pc);
		} break;

		case opcodes::BCOND:
		{
			execute_bcond(_pc);
		} break;

		case opcodes::J:
		{
			jump(jmp_instr);
		} break;

		case opcodes::JAL:
		{
			jump_and_link(jmp_instr);
		} break;

		case opcodes::BEQ:
		{
			branch_on_equal(imm_instr);
		} break;

		case opcodes::BNE:
		{
			branch_on_not_equal(imm_instr);
		} break;

		case opcodes::BLEZ:
		{
			branch_on_less_than_or_equal_zero(imm_instr);
		} break;

		case opcodes::BGTZ:
		{
			branch_on_greater_than_zero(imm_instr);
		} break;

		case opcodes::ADDI:
		{
			add_immediate(imm_instr);
		} break;

		case opcodes::ADDIU:
		{
			add_immediate_unsigned(imm_instr);
		} break;

		case opcodes::SLTI:
		{
			set_on_less_than_immediate(imm_instr);
		} break;

		case opcodes::SLTIU:
		{
			set_on_less_than_unsigned_immediate(imm_instr);
		} break;

		case opcodes::ANDI:
		{
			and_immediate(imm_instr);
		} break;

		case opcodes::ORI:
		{
			or_immediate(imm_instr);
		} break;

		case opcodes::XORI:
		{
			xor_immediate(imm_instr);
		} break;

		case opcodes::LUI:
		{
			load_upper_immediate(imm_instr);
		} break;

		case opcodes::LB:
		{
			load_byte(imm_instr);
		} break;

		case opcodes::LH:
		{
			load_halfword(imm_instr);
		} break;

		case opcodes::LWL:
		{
			load_word_left(imm_instr);
		} break;

		case opcodes::LW:
		{
			load_word(imm_instr);
		} break;

		case opcodes::LBU:
		{
			load_byte_unsigned(imm_instr);
		} break;

		case opcodes::LHU:
		{
			load_halfword_unsigned(imm_instr);
		} break;

		case opcodes::LWR:
		{
			load_word_right(imm_instr);
		} break;

		case opcodes::SB:
		{
			store_byte(imm_instr);
		} break;

		case opcodes::SH:
		{
			store_halfword(imm_instr);
		} break;

		case opcodes::SWL:
		{
			store_word_left(imm_instr);
		} break;

		case opcodes::SW:
		{
			store_word(imm_instr);
		} break;

		case opcodes::SWR:
		{
			store_word_right(imm_instr);
		} break;

		default:
			throw std::logic_error("unimplemented opcode");
	}
}

void Cpu::execute_special(unsigned int _pc)
{
	unsigned int instruction = *ram->get_word(_pc);
	RegisterInstruction reg_instr(instruction);
	special_funcs func = static_cast<special_funcs>(reg_instr.funct);

	switch (func) {
		case special_funcs::ADD:
		{
			add(reg_instr);
		} break;

		case special_funcs::ADDU:
		{
			add_unsigned(reg_instr);
		} break;

		case special_funcs::AND:
		{
			and(reg_instr);
		} break;

		case special_funcs::BREAK:
		{
			breakpoint();
		} break;

		case special_funcs::DIV:
		{
			div(reg_instr);
		} break;

		case special_funcs::DIVU:
		{
			div_unsigned(reg_instr);
		} break;

		case special_funcs::JALR:
		{
			jump_and_link_register(reg_instr);
		} break;

		case special_funcs::JR:
		{
			jump_register(reg_instr);
		} break;

		case special_funcs::MFHI:
		{
			move_from_hi(reg_instr);
		} break;

		case special_funcs::MFLO:
		{
			move_from_lo(reg_instr);
		} break;

		case special_funcs::MTHI:
		{
			move_to_hi(reg_instr);
		} break;

		case special_funcs::MTLO:
		{
			move_to_lo(reg_instr);
		} break;

		case special_funcs::MULT:
		{
			mult(reg_instr);
		} break;

		case special_funcs::MULTU:
		{
			mult_unsigned(reg_instr);
		} break;

		case special_funcs::NOR:
		{
			nor(reg_instr);
		} break;

		case special_funcs::OR:
		{
			or(reg_instr);
		} break;

		case special_funcs::SLL:
		{
			shift_left_logical_variable(reg_instr);
		} break;

		case special_funcs::SLLV:
		{
			shift_left_logical_variable(reg_instr);
		} break;

		case special_funcs::SLT:
		{
			set_on_less_than(reg_instr);
		} break;

		case special_funcs::SLTU:
		{
			set_on_less_than_unsigned(reg_instr);
		} break;

		case special_funcs::SRA:
		{
			shift_right_arithmetic(reg_instr);
		} break;

		case special_funcs::SRAV:
		{
			shift_right_arithmetic_variable(reg_instr);
		} break;

		case special_funcs::SRL:
		{
			shift_right_logical(reg_instr);
		} break;

		case special_funcs::SRLV:
		{
			shift_right_arithmetic_variable(reg_instr);
		} break;

		case special_funcs::SUB:
		{
			sub(reg_instr);
		} break;

		case special_funcs::SUBU:
		{
			sub_unsigned(reg_instr);
		} break;

		case special_funcs::SYSCALL:
		{
			system_call();
		} break;

		case special_funcs::XOR:
		{
			xor(reg_instr);
		} break;
	}
}

void Cpu::execute_bcond(unsigned int _pc)
{
	unsigned int instruction = *ram->get_word(_pc);
	ImmediateInstruction imm_instr(instruction);
	bconds cond = static_cast<bconds>(imm_instr.rt);

	switch (cond)
	{
		case bconds::BLTZ:
		{
			branch_on_less_than_zero(imm_instr);
		} break;

		case bconds::BGEZ:
		{
			branch_on_less_than_zero(imm_instr);
		} break;

		case bconds::BLTZAL:
		{
			branch_on_less_than_zero_and_link(imm_instr);
		} break;

		case bconds::BGEZAL:
		{
			branch_on_greater_than_or_equal_zero_and_link(imm_instr);
		} break;
	}
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

unsigned int Cpu::get_immediate_base_addr(const ImmediateInstruction& instr)
{
	return (short)instr.immediate + (int)get_register(instr.rs);
}

// load/store
// LB rt, offset(base)
void Cpu::load_byte(const ImmediateInstruction& instr) 
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned char value = *ram->get_byte(addr);

	set_register(instr.rt, value);
}

// LBU rt, offset(base)
void Cpu::load_byte_unsigned(const ImmediateInstruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned char value = *ram->get_byte(addr);

	set_register(instr.rt, value);
}

// LH rt, offset(base)
void Cpu::load_halfword(const ImmediateInstruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned short value = *ram->get_halfword(addr);

	set_register(instr.rt, value);
}

// LHU rt, offset(base)
void Cpu::load_halfword_unsigned(const ImmediateInstruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned short value = *ram->get_halfword(addr);

	set_register(instr.rt, value);
}

// LW rt, offset(base)
void Cpu::load_word(const ImmediateInstruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned int value = *ram->get_word(addr);

	set_register(instr.rt, value);
}

// LWL rt, offset(base)
void Cpu::load_word_left(const ImmediateInstruction& instr)
{
	throw std::logic_error("not implemented");
}

// LWR rt, offset(base)
void Cpu::load_word_right(const ImmediateInstruction& instr)
{
	throw std::logic_error("not implemented");
}

// SB rt, offset(base)
void Cpu::store_byte(const ImmediateInstruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	*ram->get_byte(addr) = get_register(instr.rt);
}

// SH rt, offset(base)
void Cpu::store_halfword(const ImmediateInstruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	*ram->get_halfword(addr) = get_register(instr.rt);
}

// SW rt, offset(base)
void Cpu::store_word(const ImmediateInstruction& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	*ram->get_word(addr) = get_register(instr.rt);
}

// SWL rt, offset(base)
void Cpu::store_word_left(const ImmediateInstruction& instr)
{
	throw std::logic_error("not implemented");
}

// SWR rt, offset(base)
void Cpu::store_word_right(const ImmediateInstruction& instr)
{
	throw std::logic_error("not implemented");
}

// ALU immediate operations
// ADDI rt, rs, immediate
void Cpu::add_immediate(const ImmediateInstruction& instr)
{
	int value = get_register(instr.rs) + (signed)instr.immediate;
	set_register(instr.rt, value);
}

// ADDIU rt, rs, immediate
void Cpu::add_immediate_unsigned(const ImmediateInstruction& instr)
{
	int value = get_register(instr.rs) + (signed)instr.immediate;
	set_register(instr.rt, value);
}

// SLTI rt, rs, immediate
void Cpu::set_on_less_than_immediate(const ImmediateInstruction& instr)
{
	int rs_value = get_register(instr.rs);
	short immediate_value = instr.immediate;

	set_register(instr.rt, rs_value < immediate_value ? 1 : 0);
}

// SLTIU rt, rs, immediate
void Cpu::set_on_less_than_unsigned_immediate(const ImmediateInstruction& instr)
{
	int rs_value = get_register(instr.rs);
	short immediate_value = instr.immediate;

	set_register(instr.rt, rs_value < immediate_value ? 1 : 0);
}

// ANDI rt, rs, immediate
void Cpu::and_immediate(const ImmediateInstruction& instr)
{
	unsigned int value = (unsigned int)instr.immediate & get_register(instr.rs);
	set_register(instr.rt, value);
}

// ORI rt, rs, immediate
void Cpu::or_immediate(const ImmediateInstruction& instr)
{
	unsigned int value = (unsigned int)instr.immediate | gp_registers[instr.rs];
	set_register(instr.rt, value);
}

// XORI rt, rs, immediate
void Cpu::xor_immediate(const ImmediateInstruction& instr)
{
	unsigned int value = (unsigned int)instr.immediate ^ get_register(instr.rs);
	set_register(instr.rt, value);
}

// LUI rt, immediate
void Cpu::load_upper_immediate(const ImmediateInstruction& instr)
{
	unsigned int value = (unsigned int)(instr.immediate) << 16;
	set_register(instr.rt, value);
}

// three operand register type
// ADD rd, rs, rt
void Cpu::add(const RegisterInstruction& instr)
{
	unsigned int value = gp_registers[instr.rs] + gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// ADDU rd, rs, rt
void Cpu::add_unsigned(const RegisterInstruction& instr)
{
	unsigned int value = gp_registers[instr.rs] + gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// SUB rd, rs, rt
void Cpu::sub(const RegisterInstruction& instr)
{
	unsigned int value = gp_registers[instr.rs] - gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// SUBU rd, rs, rt
void Cpu::sub_unsigned(const RegisterInstruction& instr)
{
	unsigned int value = gp_registers[instr.rs] - gp_registers[instr.rt];
	set_register(instr.rd, value);
}

// SLT rd, rs, rt
void Cpu::set_on_less_than(const RegisterInstruction& instr)
{
	int rt_value = get_register(instr.rt);
	int rs_value = get_register(instr.rs);

	set_register(instr.rd, rs_value < rt_value ? 1 : 0);
}

// SLTU rd, rs, rt
void Cpu::set_on_less_than_unsigned(const RegisterInstruction& instr)
{
	unsigned int rt_value = get_register(instr.rt);
	unsigned int rs_value = get_register(instr.rs);

	set_register(instr.rd, rs_value < rt_value ? 1 : 0);
}

// AND rd, rs, rt
void Cpu::and(const RegisterInstruction& instr)
{
	unsigned int value = get_register(instr.rs) & get_register(instr.rt);
	set_register(instr.rd, value);
}

// OR rd, rs, rt
void Cpu::or(const RegisterInstruction& instr)
{
	unsigned int value = get_register(instr.rs) | get_register(instr.rt);
	set_register(instr.rd, value);
}

// XOR rd, rs, rt
void Cpu::xor (const RegisterInstruction& instr)
{
	unsigned int value = get_register(instr.rs) ^ get_register(instr.rt);
	set_register(instr.rd, value);
}

// NOR rd, rs, rt
void Cpu::nor(const RegisterInstruction& instr)
{
	unsigned int value = ~(get_register(instr.rs) | get_register(instr.rt));
	set_register(instr.rd, value);
}

// shift operations
// SLL rd, rt, shamt
void Cpu::shift_left_logical(const RegisterInstruction& instr)
{
	unsigned int value = get_register(instr.rt) << instr.shamt;
	set_register(instr.rd, value);
}

// SRL rd, rt, shamt
void Cpu::shift_right_logical(const RegisterInstruction& instr)
{
	unsigned int value = get_register(instr.rt) >> instr.shamt;
	set_register(instr.rd, value);
}

// SRA rd, rt, shamt
void Cpu::shift_right_arithmetic(const RegisterInstruction& instr)
{
	unsigned int value = (int)(get_register(instr.rt)) >> instr.shamt;
	set_register(instr.rd, value);
}

// SLLV rd, rt, rs
void Cpu::shift_left_logical_variable(const RegisterInstruction& instr)
{
	unsigned int value = get_register(instr.rs) << (0x1F & get_register(instr.rt));
	set_register(instr.rd, value);
}

// SRLV rd, rt, rs
void Cpu::shift_right_logical_variable(const RegisterInstruction& instr)
{
	unsigned int value = get_register(instr.rs) >> (0x1F & get_register(instr.rt));
	set_register(instr.rd, value);
}

// SRAV rd, rt, rs
void Cpu::shift_right_arithmetic_variable(const RegisterInstruction& instr)
{
	unsigned int value = (int)get_register(instr.rs) >> (0x1F & get_register(instr.rt));
	set_register(instr.rd, value);
}

// multiply/divide
// MULT rs, rt
void Cpu::mult(const RegisterInstruction& instr)
{
	long long result = (int)(get_register(instr.rs)) * (int)(get_register(instr.rt));
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// MULTU rs, rt
void Cpu::mult_unsigned(const RegisterInstruction& instr)
{
	unsigned long long result = get_register(instr.rs) * get_register(instr.rt);
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// DIV rs, rt
void Cpu::div(const RegisterInstruction& instr)
{
	hi = (int)get_register(instr.rs) / (int)get_register(instr.rt);
	lo = (int)get_register(instr.rs) % (int)get_register(instr.rt);
}

// DIVU rs, rt
void Cpu::div_unsigned(const RegisterInstruction& instr)
{
	hi = get_register(instr.rs) / get_register(instr.rt);
	lo = get_register(instr.rs) % get_register(instr.rt);
}

// MFHI rd
void Cpu::move_from_hi(const RegisterInstruction& instr)
{
	set_register(instr.rd, hi);
}

// MFLO rd
void Cpu::move_from_lo(const RegisterInstruction& instr)
{
	set_register(instr.rd, lo);
}

// MTHI rd
void Cpu::move_to_hi(const RegisterInstruction& instr)
{
	hi = get_register(instr.rs);
}

// MTLO rd
void Cpu::move_to_lo(const RegisterInstruction& instr)
{
	lo = get_register(instr.rs);
}

// jump instructions
// J target
void Cpu::jump(const JumpInstruction& instr)
{
	unsigned int _pc = pc;
	execute(_pc + 4);
	pc = (unsigned int)instr.target << 2 & (0xF0000000 & _pc);
}

// JAL target
void Cpu::jump_and_link(const JumpInstruction& instr)
{
	unsigned int _pc = pc;
	execute(_pc + 4);
	gp_registers[31] = _pc + 8;
	pc = (unsigned int)instr.target << 2 & (0xF0000000 & _pc);
}

// JR rs
void Cpu::jump_register(const RegisterInstruction& instr)
{
	execute(pc + 4);
	pc = get_register(instr.rs);
}

// JALR rs, rd
void Cpu::jump_and_link_register(const RegisterInstruction& instr)
{
	unsigned int _pc = pc;
	execute(_pc + 4);
	gp_registers[31] = _pc + 8;
	pc = get_register(instr.rs);
}

// branch instructions
// BEQ rs, rt, offset
void Cpu::branch_on_equal(const ImmediateInstruction& instr)
{
	if (get_register(instr.rt) == get_register(instr.rs)) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BNE rs, rt, offset
void Cpu::branch_on_not_equal(const ImmediateInstruction& instr)
{
	if (get_register(instr.rt) != get_register(instr.rs)) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BLEZ rs, offset
void Cpu::branch_on_less_than_or_equal_zero(const ImmediateInstruction& instr)
{
	if (get_register(instr.rs) <= 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BGTZ rs, offset
void Cpu::branch_on_greater_than_zero(const ImmediateInstruction& instr)
{
	if (get_register(instr.rs) > 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BLTZ rs, offset
void Cpu::branch_on_less_than_zero(const ImmediateInstruction& instr)
{
	if (get_register(instr.rs) < 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BGEZ rs, offset
void Cpu::branch_on_greater_than_or_equal_zero(const ImmediateInstruction& instr)
{
	if (get_register(instr.rs) <= 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BLTZAL rs, offset
void Cpu::branch_on_less_than_zero_and_link(const ImmediateInstruction& instr)
{
	if (get_register(instr.rs) < 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		gp_registers[31] = delay_addr + 4;
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BGEZAL rs, offset
void Cpu::branch_on_greater_than_or_equal_zero_and_link(const ImmediateInstruction& instr)
{
	if (get_register(instr.rs) >= 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		gp_registers[31] = delay_addr + 4;
		pc = delay_addr + (int)instr.immediate << 2;
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
void Cpu::load_word_to_cop(const ImmediateInstruction& instr)
{
	unsigned int cop_number = instr.op & 0x3f;
	coprocessors[cop_number]->load_word_to_cop(instr);
}

// SWCz rt, offset(base)
void Cpu::store_word_from_cop(const ImmediateInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	coprocessors[cop_number]->store_word_from_cop(instr);
}

// MTCz rt, rd
void Cpu::move_to_cop(const RegisterInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	coprocessors[cop_number]->move_to_cop(instr);
}

// MFCz rt, rd
void Cpu::move_from_cop(const RegisterInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	coprocessors[cop_number]->move_from_cop(instr);
}

// CTCz rt, rd
void Cpu::move_control_to_cop(const RegisterInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	coprocessors[cop_number]->move_control_to_cop(instr);
}

// CFCz rt, rd
void Cpu::move_control_from_cop(const RegisterInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	coprocessors[cop_number]->move_control_from_cop(instr);
}

// COPz cofun
void Cpu::move_control_to_cop_fun(const RegisterInstruction& instr)
{
	unsigned int cop_number = instr.op & 0x3f;
	coprocessors[cop_number]->move_control_to_cop_fun(instr);
}

// system control
// MTC0 rt, rd
void Cpu::move_to_cp0(const RegisterInstruction& instr)
{
	throw std::logic_error("not implemented");
}

// MFC0 rt, rd
void Cpu::move_from_cp0(const RegisterInstruction& instr)
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