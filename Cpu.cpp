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
void Cpu::load_byte(const ImmediateInstruction& instr){}
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

// ALU immediate operations
void Cpu::add_immediate(const ImmediateInstruction& instr){}
void Cpu::add_immediate_unsigned(const ImmediateInstruction& instr){}
void Cpu::set_on_less_than_immediate(const ImmediateInstruction& instr){}
void Cpu::set_on_less_than_unsigned_immediate(const ImmediateInstruction& instr){}
void Cpu::and_immediate(const ImmediateInstruction& instr){}
void Cpu::or_immediate(const ImmediateInstruction& instr){}
void Cpu::xor_immediate(const ImmediateInstruction& instr){}
void Cpu::load_upper_immediate(const ImmediateInstruction& instr){}

// three operand register type
void Cpu::add(const RegisterInstruction& instr){}
void Cpu::add_unsigned(const RegisterInstruction& instr){}
void Cpu::sub(const RegisterInstruction& instr){}
void Cpu::sub_unsigned(const RegisterInstruction& instr){}
void Cpu::set_on_less_than(const RegisterInstruction& instr){}
void Cpu::set_on_less_than_immediate(const RegisterInstruction& instr){}
void Cpu::and(const RegisterInstruction& instr){}
void Cpu::xor(const RegisterInstruction& instr){}
void Cpu::nor(const RegisterInstruction& instr){}

// shift operations
void Cpu::shift_left_logical(const RegisterInstruction& instr){}
void Cpu::shift_right_logical(const RegisterInstruction& instr){}
void Cpu::shift_right_arithmetic(const RegisterInstruction& instr){}
void Cpu::shift_left_logical_variable(const RegisterInstruction& instr){}
void Cpu::shift_right_logical_variable(const RegisterInstruction& instr){}
void Cpu::shift_right_arithmetic_variable(const RegisterInstruction& instr){}

// multiply/divide
void Cpu::mult(const RegisterInstruction& instr){}
void Cpu::mult_unsigned(const RegisterInstruction& instr){}
void Cpu::div(const RegisterInstruction& instr){}
void Cpu::div_unsigned(const RegisterInstruction& instr){}
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