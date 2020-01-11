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


// load/store
void Cpu::load_byte(const ImmediateInstruction& instr) {}

void Cpu::load_byte_unsigned(const ImmediateInstruction& instr){}
void Cpu::load_halfword(const ImmediateInstruction& instr){}
void Cpu::load_halfword_unsigned(const ImmediateInstruction& instr){}
void Cpu::load_word(const ImmediateInstruction& instr){}
void Cpu::load_word_left(const ImmediateInstruction& instr){}
void Cpu::load_word_right(const ImmediateInstruction& instr){}

void Cpu::store_byte(const ImmediateInstruction& instr){}
void Cpu::store_halfword(const ImmediateInstruction& instr){}
void Cpu::store_word(const ImmediateInstruction& instr){}
void Cpu::store_word_left(const ImmediateInstruction& instr){}
void Cpu::store_word_right(const ImmediateInstruction& instr){}

bool is_overflow(int a, int b) {

	if (a > 0 && b > 0) {
		if (a + b < 0) {
			return true;
		}
	}
	else if (a < 0 && b < 0) {
		if (a+b >= 0) {
			return true;
		}
	}
	return false;
}

// ALU immediate operations
void Cpu::add_immediate(const ImmediateInstruction& instr)
{
	// TODO add overflow trap
	add_immediate_unsigned(instr);
}

void Cpu::add_immediate_unsigned(const ImmediateInstruction& instr)
{
	int rs_value = gp_registers[instr.rs];
	short immediate_value = instr.immediate;

	int result = rs_value + (short)instr.immediate;

	if (instr.rt != 0)
	{
		gp_registers[instr.rt] = result;
	}
}

void Cpu::set_on_less_than_immediate(const ImmediateInstruction& instr)
{
	// TODO add overflow trap
	set_on_less_than_unsigned_immediate(instr);
}

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

void Cpu::and_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)instr.immediate & gp_registers[instr.rs];
	}
}

void Cpu::or_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)instr.immediate | gp_registers[instr.rs];
	}
}

void Cpu::xor_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)instr.immediate ^ gp_registers[instr.rs];
	}
}

void Cpu::load_upper_immediate(const ImmediateInstruction& instr)
{
	if (instr.rt != 0) {
		gp_registers[instr.rt] = (unsigned int)(instr.immediate) << 16;
	}
}

// three operand register type
void Cpu::add(const RegisterInstruction& instr)
{
	// TODO overflow
	add_unsigned(instr);
}

void Cpu::add_unsigned(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] + gp_registers[instr.rt];
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

void Cpu::sub(const RegisterInstruction& instr)
{
	// TODO overflow
	sub_unsigned(instr);
}

void Cpu::sub_unsigned(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] - gp_registers[instr.rt];
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

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
void Cpu::and(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = gp_registers[instr.rs] & gp_registers[instr.rt];
	}
}

void Cpu::or(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = gp_registers[instr.rs] | gp_registers[instr.rt];
	}
}

void Cpu::xor (const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = gp_registers[instr.rs] ^ gp_registers[instr.rt];
	}
}

void Cpu::nor(const RegisterInstruction& instr)
{
	if (instr.rd != 0) {
		gp_registers[instr.rd] = ~(gp_registers[instr.rs] | gp_registers[instr.rt]);
	}
}

// shift operations
void Cpu::shift_left_logical(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rt] << instr.shamt;
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

void Cpu::shift_right_logical(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rt] >> instr.shamt;
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

void Cpu::shift_right_arithmetic(const RegisterInstruction& instr)
{
	unsigned int result = (int)(gp_registers[instr.rt]) >> instr.shamt;
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

void Cpu::shift_left_logical_variable(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] << (0x1F & gp_registers[instr.rt]);
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

void Cpu::shift_right_logical_variable(const RegisterInstruction& instr)
{
	unsigned int result = gp_registers[instr.rs] >> (0x1F & gp_registers[instr.rt]);
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}
void Cpu::shift_right_arithmetic_variable(const RegisterInstruction& instr)
{
	unsigned int result = (int)(gp_registers[instr.rs]) << (0x1F & gp_registers[instr.rt]);
	if (instr.rd != 0) {
		gp_registers[instr.rd] = result;
	}
}

// multiply/divide
void Cpu::mult(const RegisterInstruction& instr)
{
	long long result = (int)(gp_registers[instr.rs]) * (int)(gp_registers[instr.rt]);
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

void Cpu::mult_unsigned(const RegisterInstruction& instr)
{
	unsigned long long result = gp_registers[instr.rs] * gp_registers[instr.rt];
	// TODO double check correct registers
	hi = result >> 32;
	lo = result & 0xFFFF;
}

void Cpu::div(const RegisterInstruction& instr)
{
	hi = (int)gp_registers[instr.rs] / (int)gp_registers[instr.rt];
	lo = (int)gp_registers[instr.rs] % (int)gp_registers[instr.rt];
}

void Cpu::div_unsigned(const RegisterInstruction& instr)
{
	hi = gp_registers[instr.rs] / gp_registers[instr.rt];
	lo = gp_registers[instr.rs] % gp_registers[instr.rt];
}

void Cpu::move_from_hi(const RegisterInstruction& instr){}
void Cpu::move_from_lo(const RegisterInstruction& instr){}
void Cpu::move_to_hi(const RegisterInstruction& instr){}
void Cpu::move_to_lo(const RegisterInstruction& instr){}

// jump instructions
void Cpu::jump(const JumpInstruction& instr){}
void Cpu::jump_and_link(const JumpInstruction& instr){}
void Cpu::jump_register(const RegisterInstruction& instr){}
void Cpu::jump_and_link_register(const RegisterInstruction& instr){}

// branch instructions
void Cpu::branch_on_equal(const ImmediateInstruction& instr){}
void Cpu::branch_on_not_equal(const ImmediateInstruction& instr){}
void Cpu::branch_on_less_than_or_equal_zero(const ImmediateInstruction& instr){}
void Cpu::branch_on_greater_than_zero(const ImmediateInstruction& instr){}
void Cpu::branch_on_less_than_zero(const ImmediateInstruction& instr){}
void Cpu::branch_on_greater_than_or_equal_zero(const ImmediateInstruction& instr){}
void Cpu::branch_on_less_than_zero_and_link(const ImmediateInstruction& instr){}
void Cpu::branch_on_greater_than_or_equal_zero_and_link(const ImmediateInstruction& instr){}

// special instructions
void Cpu::system_call(){}
void Cpu::breakpoint(){}

// co-processor instructions
void Cpu::load_word_to_cop(const ImmediateInstruction& instr){}
void Cpu::store_word_from_cop(const ImmediateInstruction& instr){}
void Cpu::move_to_cop(const RegisterInstruction& instr){}
void Cpu::move_from_cop(const RegisterInstruction& instr){}
void Cpu::move_control_to_cop(const RegisterInstruction& instr){}
void Cpu::move_control_from_cop(const RegisterInstruction& instr){}
void Cpu::move_control_to_cop_fun(const RegisterInstruction& instr){}

// system control
void Cpu::move_to_cp0(const RegisterInstruction& instr){}
void Cpu::move_from_cp0(const RegisterInstruction& instr){}
void Cpu::read_indexed_tlb(){}
void Cpu::write_indexed_tlb(){}
void Cpu::write_random_tlb(){}
void Cpu::probe_tlb_for_matching_entry(){}
void Cpu::restore_from_exception(){}