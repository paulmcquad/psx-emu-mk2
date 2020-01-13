#include "Cpu.hpp"

constexpr unsigned int BIOS_START = 0xbfc00000;

void Cpu::reset()
{
	pc = BIOS_START;
}

void Cpu::run()
{
	pc += 4;
}

void Cpu::execute(unsigned int _pc)
{

}


// load/store
// LB rt, offset(base)
void Cpu::load_byte(const ImmediateInstruction& instr) 
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	char result = (char)ram[addr];
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (int)result;
	}
}

// LBU rt, offset(base)
void Cpu::load_byte_unsigned(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	unsigned char& result = ram[addr];
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)result;
	}
}

// LH rt, offset(base)
void Cpu::load_halfword(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	short& value = (short&)ram[addr];

	if (instr.rt != 0) {
		gp_registers[instr.rt] = (int)value;
	}
}

// LHU rt, offset(base)
void Cpu::load_halfword_unsigned(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	unsigned short& value = (unsigned short&)ram[addr];

	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)value;
	}
}

// LW rt, offset(base)
void Cpu::load_word(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	int& result = (int&)ram[addr];
	if (instr.rt != 0) {
		gp_registers[instr.rt] = result;
	}
}

// LWL rt, offset(base)
void Cpu::load_word_left(const ImmediateInstruction& instr)
{

}

// LWR rt, offset(base)
void Cpu::load_word_right(const ImmediateInstruction& instr)
{

}

// SB rt, offset(base)
void Cpu::store_byte(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	ram[addr] = gp_registers[instr.rs];
}

// SH rt, offset(base)
void Cpu::store_halfword(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	((short&)ram[addr]) = gp_registers[instr.rs];
}

// SW rt, offset(base)
void Cpu::store_word(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	((int&)ram[addr]) = gp_registers[instr.rs];
}

// SWL rt, offset(base)
void Cpu::store_word_left(const ImmediateInstruction& instr)
{

}

// SWR rt, offset(base)
void Cpu::store_word_right(const ImmediateInstruction& instr)
{

}

// ALU immediate operations
// ADDI rt, rs, immediate
void Cpu::add_immediate(const ImmediateInstruction& instr)
{
	int result = gp_registers[instr.rs] + (signed)instr.immediate;
	if (instr.rt != 0) {
		gp_registers[instr.rt] = result;
	}
}

// ADDIU rt, rs, immediate
void Cpu::add_immediate_unsigned(const ImmediateInstruction& instr)
{
	int result = gp_registers[instr.rs] + (signed)instr.immediate;
	if (instr.rt != 0) {
		gp_registers[instr.rt] = result;
	}
}

// SLTI rt, rs, immediate
void Cpu::set_on_less_than_immediate(const ImmediateInstruction& instr)
{
	// TODO add overflow trap
	set_on_less_than_unsigned_immediate(instr);
}

// SLTIU rt, rs, immediate
void Cpu::set_on_less_than_unsigned_immediate(const ImmediateInstruction& instr)
{
	int rs_value = gp_registers[instr.rs];
	short immediate_value = instr.immediate;

	if (instr.rt != 0)
	{
		if (rs_value < immediate_value) {
			gp_registers[instr.rs] = 1;
		}
		else
		{
			gp_registers[instr.rs] = 0;
		}
	}
}

// ANDI rt, rs, immediate
void Cpu::and_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)instr.immediate & gp_registers[instr.rs];
	}
}

// ORI rt, rs, immediate
void Cpu::or_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)instr.immediate | gp_registers[instr.rs];
	}
}

// XORI rt, rs, immediate
void Cpu::xor_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)instr.immediate ^ gp_registers[instr.rs];
	}
}

// LUI rt, immediate
void Cpu::load_upper_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)(instr.immediate) << 16;
	}
}

// three operand register type
// ADD rd, rs, rt
void Cpu::add(const RegisterInstruction& instr)
{
	// TODO overflow
	add_unsigned(instr);
}

// ADDU rd, rs, rt
void Cpu::add_unsigned(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] + gp_registers[instr.rt];
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// SUB rd, rs, rt
void Cpu::sub(const RegisterInstruction& instr)
{
	// TODO overflow
	sub_unsigned(instr);
}

// SUBU rd, rs, rt
void Cpu::sub_unsigned(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] - gp_registers[instr.rt];
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// SLT rd, rs, rt
void Cpu::set_on_less_than(const RegisterInstruction& instr)
{
	int rt_value = gp_registers[instr.rt];
	int rs_value = gp_registers[instr.rs];
	if (instr.rd != 0) {
		if (rs_value < rt_value) {
			gp_registers[instr.rd] = 1;
		} 
		else {
			gp_registers[instr.rd] = 0;
		}
	}
}

// SLTU rd, rs, rt
void Cpu::set_on_less_than_unsigned(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		if (gp_registers[instr.rs] < gp_registers[instr.rt]) {
			gp_registers[instr.rd] = 1;
		}
		else {
			gp_registers[instr.rd] = 0;
		}
	}
}

// AND rd, rs, rt
void Cpu::and(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = gp_registers[instr.rs] & gp_registers[instr.rt];
	}
}

// OR rd, rs, rt
void Cpu::or(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = gp_registers[instr.rs] | gp_registers[instr.rt];
	}
}

// XOR rd, rs, rt
void Cpu::xor (const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = gp_registers[instr.rs] ^ gp_registers[instr.rt];
	}
}


// NOR rd, rs, rt
void Cpu::nor(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = ~(gp_registers[instr.rs] | gp_registers[instr.rt]);
	}
}

// shift operations
// SLL rd, rt, shamt
void Cpu::shift_left_logical(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rt] << instr.shamt;
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// SRL rd, rt, shamt
void Cpu::shift_right_logical(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rt] >> instr.shamt;
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}


// SRA rd, rt, shamt
void Cpu::shift_right_arithmetic(const RegisterInstruction& instr)
{
	unsigned int result = (int)(gp_registers[instr.rt]) >> instr.shamt;
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// SLLV rd, rt, rs
void Cpu::shift_left_logical_variable(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] << (0x1F & gp_registers[instr.rt]);
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// SRLV rd, rt, rs
void Cpu::shift_right_logical_variable(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] >> (0x1F & gp_registers[instr.rt]);
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// SRAV rd, rt, rs
void Cpu::shift_right_arithmetic_variable(const RegisterInstruction& instr)
{
	unsigned int result = (int)(gp_registers[instr.rs]) << (0x1F & gp_registers[instr.rt]);
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// multiply/divide
// MULT rs, rt
void Cpu::mult(const RegisterInstruction& instr)
{
	long long result = (int)(gp_registers[instr.rs]) * (int)(gp_registers[instr.rt]);
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// MULTU rs, rt
void Cpu::mult_unsigned(const RegisterInstruction& instr)
{
	unsigned long long result = gp_registers[instr.rs] * gp_registers[instr.rt];
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// DIV rs, rt
void Cpu::div(const RegisterInstruction& instr)
{
	hi = (int)gp_registers[instr.rs] / (int)gp_registers[instr.rt];
	lo = (int)gp_registers[instr.rs] % (int)gp_registers[instr.rt];
}

// DIVU rs, rt
void Cpu::div_unsigned(const RegisterInstruction& instr)
{
	hi = gp_registers[instr.rs] / gp_registers[instr.rt];
	lo = gp_registers[instr.rs] % gp_registers[instr.rt];
}

// MFHI rd
void Cpu::move_from_hi(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = hi;
	}
}

// MFLO rd
void Cpu::move_from_lo(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = lo;
	}
}

// MTHI rd
void Cpu::move_to_hi(const RegisterInstruction& instr)
{
	hi = gp_registers[instr.rs];
}

// MTLO rd
void Cpu::move_to_lo(const RegisterInstruction& instr)
{
	lo = gp_registers[instr.rs];
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
	pc = gp_registers[instr.rs];
}

// JALR rs, rd
void Cpu::jump_and_link_register(const RegisterInstruction& instr)
{
	unsigned int _pc = pc;
	execute(_pc + 4);
	gp_registers[31] = _pc + 8;
	pc = gp_registers[instr.rs];
}

// branch instructions
// BEQ rs, rt, offset
void Cpu::branch_on_equal(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rt] == gp_registers[instr.rs]) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BNE rs, rt, offset
void Cpu::branch_on_not_equal(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rt] != gp_registers[instr.rs]) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BLEZ rs, offset
void Cpu::branch_on_less_than_or_equal_zero(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rs] <= 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BGTZ rs, offset
void Cpu::branch_on_greater_than_zero(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rs] > 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BLTZ rs, offset
void Cpu::branch_on_less_than_zero(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rs] < 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BGEZ rs, offset
void Cpu::branch_on_greater_than_or_equal_zero(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rs] <= 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BLTZAL rs, offset
void Cpu::branch_on_less_than_zero_and_link(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rs] < 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		gp_registers[31] = delay_addr + 4;
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// BGEZAL rs, offset
void Cpu::branch_on_greater_than_or_equal_zero_and_link(const ImmediateInstruction& instr)
{
	if (gp_registers[instr.rs] >= 0) {
		unsigned int delay_addr = pc + 4;
		execute(pc + 4);
		gp_registers[31] = delay_addr + 4;
		pc = delay_addr + (int)instr.immediate << 2;
	}
}

// special instructions
// SYSCALL
void Cpu::system_call(){}

// BREAK
void Cpu::breakpoint(){}

// co-processor instructions
// LWCz rt, offset(base)
void Cpu::load_word_to_cop(const ImmediateInstruction& instr){}

// SWCz rt, offset(base)
void Cpu::store_word_from_cop(const ImmediateInstruction& instr){}

// MTCz rt, rd
void Cpu::move_to_cop(const RegisterInstruction& instr){}

// MFCz rt, rd
void Cpu::move_from_cop(const RegisterInstruction& instr){}

// CTCz rt, rd
void Cpu::move_control_to_cop(const RegisterInstruction& instr){}

// CFCz rt, rd
void Cpu::move_control_from_cop(const RegisterInstruction& instr){}

// COPz cofun
void Cpu::move_control_to_cop_fun(const RegisterInstruction& instr){}

// system control
// MTC0 rt, rd
void Cpu::move_to_cp0(const RegisterInstruction& instr){}

// MFC0 rt, rd
void Cpu::move_from_cp0(const RegisterInstruction& instr){}

// TLBR
void Cpu::read_indexed_tlb(){}

// TLBWI
void Cpu::write_indexed_tlb(){}

// TLBWR
void Cpu::write_random_tlb(){}

// TLBP
void Cpu::probe_tlb_for_matching_entry(){}

//RFE
void Cpu::restore_from_exception(){}