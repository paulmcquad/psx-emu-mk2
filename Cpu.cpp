#include "Cpu.hpp"
#include "InstructionTypes.hpp"

constexpr unsigned int BIOS_START = 0xbfc00000;

Cpu::Cpu(std::shared_ptr<Ram> ram)
{
	coprocessors[0] = nullptr;
	coprocessors[2] = nullptr;
}

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

// load/store
// LB rt, offset(base)
void Cpu::load_byte(const ImmediateInstruction& instr) 
{
	unsigned int addr = (short)instr.immediate + (int)get_register(instr.rs);
	unsigned char value = *ram->get_byte(addr);

	set_register(instr.rt, value);
}

// LBU rt, offset(base)
void Cpu::load_byte_unsigned(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)get_register(instr.rs);
	unsigned char value = *ram->get_byte(addr);

	set_register(instr.rt, value);
}

// LH rt, offset(base)
void Cpu::load_halfword(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)get_register(instr.rs);
	unsigned short value = *ram->get_halfword(addr);

	set_register(instr.rt, value);
}

// LHU rt, offset(base)
void Cpu::load_halfword_unsigned(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)get_register(instr.rs);
	unsigned short value = *ram->get_halfword(addr);

	set_register(instr.rt, value);
}

// LW rt, offset(base)
void Cpu::load_word(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)get_register(instr.rs);
	unsigned int value = *ram->get_word(addr);

	set_register(instr.rt, value);
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
	unsigned int addr = (short)instr.immediate + (int)get_register(instr.rs);
	*ram->get_byte(addr) = get_register(instr.rt);
}

// SH rt, offset(base)
void Cpu::store_halfword(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	*ram->get_halfword(addr) = get_register(instr.rt);
}

// SW rt, offset(base)
void Cpu::store_word(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	*ram->get_word(addr) = get_register(instr.rt);
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
void Cpu::system_call(){}

// BREAK
void Cpu::breakpoint(){}

// co-processor instructions
// LWCz rt, offset(base)
void Cpu::load_word_to_cop(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	unsigned int& value = (unsigned int&)ram[addr];

	int cop_number = instr.op & 0x3f;

	if (cop_number == 0)
	{
		cp0_registers[instr.rt] = value;
	}
	else if (cop_number == 2)
	{
		cp2_data_registers[instr.rt] = value;
	}
}

// SWCz rt, offset(base)
void Cpu::store_word_from_cop(const ImmediateInstruction& instr)
{
	unsigned int addr = (short)instr.immediate + (int)gp_registers[instr.rs];
	int cop_number = instr.op & 0x3f;

	if (cop_number == 0)
	{
		(unsigned int&)(ram[addr]) = cp0_registers[instr.rt];
	}
	else if (cop_number == 2)
	{
		(unsigned int&)(ram[addr]) =  cp2_data_registers[instr.rt];
	}
}

// MTCz rt, rd
void Cpu::move_to_cop(const RegisterInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	if (cop_number == 0)
	{
		cp0_registers[instr.rd] = gp_registers[instr.rt];
	}
	else if (cop_number == 2)
	{
		cp2_data_registers[instr.rd] = gp_registers[instr.rt];
	}
}

// MFCz rt, rd
void Cpu::move_from_cop(const RegisterInstruction& instr)
{
	if (instr.rt != 0) {
		int cop_number = instr.op & 0x3f;
		if (cop_number == 0)
		{
			gp_registers[instr.rt] = cp0_registers[instr.rd];
		}
		else if (cop_number == 2)
		{
			gp_registers[instr.rt] = cp2_data_registers[instr.rd];
		}
	}
}

// CTCz rt, rd
void Cpu::move_control_to_cop(const RegisterInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	if (cop_number == 2)
	{
		cp2_control_registers[instr.rd] = gp_registers[instr.rt];
	}
}

// CFCz rt, rd
void Cpu::move_control_from_cop(const RegisterInstruction& instr)
{
	int cop_number = instr.op & 0x3f;
	if (instr.rt !=0 && cop_number == 2)
	{
		gp_registers[instr.rt] = cp2_control_registers[instr.rd];
	}
}

// COPz cofun
void Cpu::move_control_to_cop_fun(const RegisterInstruction& instr)
{
	// todo
}

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