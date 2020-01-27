#include <assert.h>
#include <iostream>
#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "Coprocessor0.hpp"
#include "Coprocessor2.hpp"
#include "InstructionEnums.hpp"
#include "Exceptions.hpp"

constexpr unsigned int BIOS_START = 0xbfc00000;

Cpu::Cpu()
{
	main_instructions[cpu_instructions::ADDI] = &Cpu::add_immediate;
	main_instructions[cpu_instructions::ADDIU] = &Cpu::add_immediate_unsigned;
	main_instructions[cpu_instructions::ANDI] = &Cpu::and_immediate;
	main_instructions[cpu_instructions::BCOND] = &Cpu::execute_bcond;
	main_instructions[cpu_instructions::BEQ] = &Cpu::branch_on_equal;
	main_instructions[cpu_instructions::BGTZ] = &Cpu::branch_on_greater_than_zero;
	main_instructions[cpu_instructions::BLEZ] = &Cpu::branch_on_less_than_or_equal_zero;
	main_instructions[cpu_instructions::BNE] = &Cpu::branch_on_not_equal;
	main_instructions[cpu_instructions::COP0] = &Cpu::execute_cop;
	main_instructions[cpu_instructions::COP2] = &Cpu::execute_cop;
	main_instructions[cpu_instructions::J] = &Cpu::jump;
	main_instructions[cpu_instructions::JAL] = &Cpu::jump_and_link;
	main_instructions[cpu_instructions::LB] = &Cpu::load_byte;
	main_instructions[cpu_instructions::LBU] = &Cpu::load_byte_unsigned;
	main_instructions[cpu_instructions::LH] = &Cpu::load_halfword;
	main_instructions[cpu_instructions::LHU] = &Cpu::load_halfword_unsigned;
	main_instructions[cpu_instructions::LUI] = &Cpu::load_upper_immediate;
	main_instructions[cpu_instructions::LW] = &Cpu::load_word;
	main_instructions[cpu_instructions::LWC0] = &Cpu::execute_cop;
	main_instructions[cpu_instructions::LWC2] = &Cpu::execute_cop;
	main_instructions[cpu_instructions::LWL] = &Cpu::load_word_left;
	main_instructions[cpu_instructions::LWR] = &Cpu::load_word_right;
	main_instructions[cpu_instructions::ORI] = &Cpu::or_immediate;
	main_instructions[cpu_instructions::SB] = &Cpu::store_byte;
	main_instructions[cpu_instructions::SH] = &Cpu::store_halfword;
	main_instructions[cpu_instructions::SLTI] = &Cpu::set_on_less_than_immediate;
	main_instructions[cpu_instructions::SLTIU] = &Cpu::set_on_less_than_unsigned_immediate;
	main_instructions[cpu_instructions::SPECIAL] = &Cpu::execute_special;
	main_instructions[cpu_instructions::SW] = &Cpu::store_word;
	main_instructions[cpu_instructions::SWC0] = &Cpu::execute_cop;
	main_instructions[cpu_instructions::SWC2] = &Cpu::execute_cop;
	main_instructions[cpu_instructions::SWL] = &Cpu::store_word_left;
	main_instructions[cpu_instructions::SWR] = &Cpu::store_word_right;
	main_instructions[cpu_instructions::XORI] = &Cpu::xor_immediate;

	special_instructions[cpu_special_funcs::ADD] = &Cpu::add;
	special_instructions[cpu_special_funcs::ADDU] = &Cpu::add_unsigned;
	special_instructions[cpu_special_funcs::AND] = &Cpu::and;
	special_instructions[cpu_special_funcs::BREAK] = &Cpu::breakpoint;
	special_instructions[cpu_special_funcs::DIV] = &Cpu::div;
	special_instructions[cpu_special_funcs::DIVU] = &Cpu::div_unsigned;
	special_instructions[cpu_special_funcs::JALR] = &Cpu::jump_and_link_register;
	special_instructions[cpu_special_funcs::JR] = &Cpu::jump_register;
	special_instructions[cpu_special_funcs::MFHI] = &Cpu::move_from_hi;
	special_instructions[cpu_special_funcs::MFLO] = &Cpu::move_from_lo;
	special_instructions[cpu_special_funcs::MTHI] = &Cpu::move_to_hi;
	special_instructions[cpu_special_funcs::MTLO] = &Cpu::move_to_lo;
	special_instructions[cpu_special_funcs::MULT] = &Cpu::mult;
	special_instructions[cpu_special_funcs::MULTU] = &Cpu::mult_unsigned;
	special_instructions[cpu_special_funcs::NOR] = &Cpu::nor;
	special_instructions[cpu_special_funcs::OR] = &Cpu::or;
	special_instructions[cpu_special_funcs::SLL] = &Cpu::shift_left_logical;
	special_instructions[cpu_special_funcs::SLLV] = &Cpu::shift_left_logical_variable;
	special_instructions[cpu_special_funcs::SLT] = &Cpu::set_on_less_than_immediate;
	special_instructions[cpu_special_funcs::SLTU] = &Cpu::set_on_less_than_unsigned;
	special_instructions[cpu_special_funcs::SRA] = &Cpu::shift_right_arithmetic;
	special_instructions[cpu_special_funcs::SRAV] = &Cpu::shift_right_arithmetic_variable;
	special_instructions[cpu_special_funcs::SRL] = &Cpu::shift_right_logical;
	special_instructions[cpu_special_funcs::SRLV] = &Cpu::shift_right_logical_variable;
	special_instructions[cpu_special_funcs::SUB] = &Cpu::sub;
	special_instructions[cpu_special_funcs::SUBU] = &Cpu::sub_unsigned;
	special_instructions[cpu_special_funcs::SYSCALL] = &Cpu::system_call;
	special_instructions[cpu_special_funcs::XOR] = &Cpu::xor;

	bcond_instructions[cpu_bconds::BGEZ] = &Cpu::branch_on_greater_than_or_equal_zero;
	bcond_instructions[cpu_bconds::BGEZAL] = &Cpu::branch_on_greater_than_or_equal_zero_and_link;
	bcond_instructions[cpu_bconds::BLTZ] = &Cpu::branch_on_less_than_zero;
	bcond_instructions[cpu_bconds::BLTZAL] = &Cpu::branch_on_less_than_zero_and_link;
}

void Cpu::init(std::shared_ptr<Ram> _ram)
{
	pc = BIOS_START;
	ram = _ram;
	next_instruction = 0x0;
	cop0 = std::make_shared<Coprocessor0>(ram, shared_from_this());
	cop2 = std::make_shared<Coprocessor2>(ram, shared_from_this());
}

void Cpu::tick()
{
	unsigned int current_pc = pc;
	unsigned int current_instruction = next_instruction;

	next_instruction = ram->load_word(current_pc);

	//std::cout << std::hex << pc << std::endl;

	pc = current_pc + 4;
	execute(current_instruction);

	std::vector<unsigned int> to_delete;
	for (auto it = delayed_loads.begin(); it != delayed_loads.end(); /* no increment */)
	{
		if (it->second.delay == 0)
		{
			if (it->first != 0) {
				gp_registers[it->first] = it->second.value;
			}
			
			it = delayed_loads.erase(it);
		}
		else
		{
			it->second.delay--;
			++it;
		}
	}
}

void Cpu::execute(unsigned int instruction)
{
	instruction_union instr;
	instr.raw = instruction;

	cpu_instructions opcode = static_cast<cpu_instructions>(instruction >> 26);
	//std::cout << "Opcode: " << std::oct << (instruction >> 26) << std::endl;

	auto fn_ptr = main_instructions[opcode];
	(this->*fn_ptr)(instr);
}

void Cpu::execute_special(const instruction_union& instr)
{
	cpu_special_funcs func = static_cast<cpu_special_funcs>(instr.register_instruction.funct);
	auto fn_ptr = special_instructions[func];
	(this->*fn_ptr)(instr);
}

void Cpu::execute_bcond(const instruction_union& instr)
{
	cpu_bconds cond = static_cast<cpu_bconds>(instr.immediate_instruction.rt);
	auto fn_ptr = bcond_instructions[cond];
	(this->*fn_ptr)(instr);
}

void Cpu::execute_cop(const instruction_union& instr)
{
	unsigned int cop_number = (instr.raw >> 26) & 0x3;
	if (cop_number == 0)
	{
		cop0->execute(instr);
	}
	else if (cop_number == 1)
	{
		cop2->execute(instr);
	}
}

unsigned int Cpu::get_register(int index) 
{
	return gp_registers[index];
}

void Cpu::set_register(int index, unsigned int value, bool delay) 
{
	if (index != 0)
	{
		if (delay)
		{
			load_delay_entry entry;
			entry.delay = 1;
			entry.value = value;
			delayed_loads[index] = entry;
		}
		else
		{
			auto it = delayed_loads.find(index);
			if (it != delayed_loads.end())
			{
				delayed_loads.erase(it);
			}
			gp_registers[index] = value;
		}
	}
}

unsigned int Cpu::get_immediate_base_addr(const instruction_union& instr)
{
	int offset = (short)instr.immediate_instruction.immediate;
	int base = get_register(instr.immediate_instruction.rs);

	unsigned int addr = offset + base;
	return addr;
}

unsigned int Cpu::get_immediate_base_addr_unsigned(const instruction_union& instr)
{
	unsigned int offset = (unsigned short)instr.immediate_instruction.immediate;
	unsigned int base = get_register(instr.immediate_instruction.rs);

	unsigned int addr = offset + base;
	return addr;
}

// load/store
// LB rt, offset(base)
void Cpu::load_byte(const instruction_union& instr) 
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned char value = ram->load_byte(addr);

	set_register(instr.immediate_instruction.rt, value, true);
}

// LBU rt, offset(base)
void Cpu::load_byte_unsigned(const instruction_union& instr)
{
	unsigned int addr = get_immediate_base_addr_unsigned(instr);
	unsigned char value = ram->load_byte(addr);

	set_register(instr.immediate_instruction.rt, value, true);
}

// LH rt, offset(base)
void Cpu::load_halfword(const instruction_union& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned short value = ram->load_halfword(addr);

	unsigned int sr = cop0->get_control_register(Coprocessor0::register_names::SR);
	bool isolate_cache = sr & 0x00010000;

	if (isolate_cache == false)
	{
		set_register(instr.immediate_instruction.rt, value, true);
	}
}

// LHU rt, offset(base)
void Cpu::load_halfword_unsigned(const instruction_union& instr)
{
	unsigned int addr = get_immediate_base_addr_unsigned(instr);
	unsigned short value = ram->load_halfword(addr);

	unsigned int sr = cop0->get_control_register(Coprocessor0::register_names::SR);
	bool isolate_cache = sr & 0x00010000;

	if (isolate_cache == false)
	{
		set_register(instr.immediate_instruction.rt, value, true);
	}
}

// LW rt, offset(base)
void Cpu::load_word(const instruction_union& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned int value = ram->load_word(addr);

	unsigned int sr = cop0->get_control_register(Coprocessor0::register_names::SR);
	bool isolate_cache = sr & 0x00010000;

	if (isolate_cache == false)
	{
		set_register(instr.immediate_instruction.rt, value, true);
	}
}

// LWL rt, offset(base)
void Cpu::load_word_left(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// LWR rt, offset(base)
void Cpu::load_word_right(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// SB rt, offset(base)
void Cpu::store_byte(const instruction_union& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned int value = get_register(instr.immediate_instruction.rt);

	unsigned int sr = cop0->get_control_register(Coprocessor0::register_names::SR);
	bool isolate_cache = sr & 0x00010000;

	if (isolate_cache == false)
	{
		ram->store_byte(addr, value);
	}
}

// SH rt, offset(base)
void Cpu::store_halfword(const instruction_union& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned int value = get_register(instr.immediate_instruction.rt);

	unsigned int sr = cop0->get_control_register(Coprocessor0::register_names::SR);
	bool isolate_cache = sr & 0x00010000;

	if (isolate_cache == false)
	{
		ram->store_halfword(addr, value);
	}
}

// SW rt, offset(base)
void Cpu::store_word(const instruction_union& instr)
{
	unsigned int addr = get_immediate_base_addr(instr);
	unsigned int value = get_register(instr.immediate_instruction.rt);

	unsigned int sr = cop0->get_control_register(Coprocessor0::register_names::SR);
	bool isolate_cache = sr & 0x00010000;

	if (isolate_cache == false)
	{
		ram->store_word(addr, value);
	}
}

// SWL rt, offset(base)
void Cpu::store_word_left(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// SWR rt, offset(base)
void Cpu::store_word_right(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// ALU immediate operations
// ADDI rt, rs, immediate
void Cpu::add_immediate(const instruction_union& instr)
{
	int immediate = (short)instr.immediate_instruction.immediate;
	int rs_value = get_register(instr.immediate_instruction.rs);

	unsigned int value = rs_value + immediate;

	if (immediate > 0 && rs_value > 0) {
		if ((int)value < 0)
		{
			throw overflow_exception();
		}
	}
	else if (immediate < 0 && rs_value < 0) {
		if ((int)value > 0)
		{
			throw overflow_exception();
		}
	}

	set_register(instr.immediate_instruction.rt, value);
}

// ADDIU rt, rs, immediate
void Cpu::add_immediate_unsigned(const instruction_union& instr)
{
	int immediate = (short)instr.immediate_instruction.immediate;
	int rs_value = get_register(instr.immediate_instruction.rs);

	unsigned int value = rs_value + immediate;

	set_register(instr.immediate_instruction.rt, value);
}

// SLTI rt, rs, immediate
void Cpu::set_on_less_than_immediate(const instruction_union& instr)
{
	int rs_value = get_register(instr.immediate_instruction.rs);
	short immediate_value = instr.immediate_instruction.immediate;

	set_register(instr.immediate_instruction.rt, rs_value < immediate_value ? 1 : 0);
}

// SLTIU rt, rs, immediate
void Cpu::set_on_less_than_unsigned_immediate(const instruction_union& instr)
{
	int rs_value = get_register(instr.immediate_instruction.rs);
	short immediate_value = instr.immediate_instruction.immediate;

	set_register(instr.immediate_instruction.rt, rs_value < immediate_value ? 1 : 0);
}

// ANDI rt, rs, immediate
void Cpu::and_immediate(const instruction_union& instr)
{
	unsigned int value = instr.immediate_instruction.immediate & get_register(instr.immediate_instruction.rs);
	set_register(instr.immediate_instruction.rt, value);
}

// ORI rt, rs, immediate
void Cpu::or_immediate(const instruction_union& instr)
{
	unsigned int rs_value = get_register(instr.immediate_instruction.rs);
	unsigned int value = rs_value | instr.immediate_instruction.immediate;
	set_register(instr.immediate_instruction.rt, value);
}

// XORI rt, rs, immediate
void Cpu::xor_immediate(const instruction_union& instr)
{
	unsigned int value = instr.immediate_instruction.immediate ^ get_register(instr.immediate_instruction.rs);
	set_register(instr.immediate_instruction.rt, value);
}

// LUI rt, immediate
void Cpu::load_upper_immediate(const instruction_union& instr)
{
	unsigned int value = instr.immediate_instruction.immediate << 16;
	set_register(instr.immediate_instruction.rt, value);
}

// three operand register type
// ADD rd, rs, rt
void Cpu::add(const instruction_union& instr)
{
	int rs_value = get_register(instr.register_instruction.rs);
	int rt_value = get_register(instr.register_instruction.rt);
	unsigned int value = rs_value + rt_value;

	if (rt_value > 0 && rs_value > 0) {
		if ((int)value < 0)
		{
			throw overflow_exception();
		}
	}
	else if (rt_value < 0 && rs_value < 0) {
		if ((int)value > 0)
		{
			throw overflow_exception();
		}
	}

	set_register(instr.register_instruction.rd, value);
}

// ADDU rd, rs, rt
void Cpu::add_unsigned(const instruction_union& instr)
{
	int rs_value = get_register(instr.register_instruction.rs);
	int rt_value = get_register(instr.register_instruction.rt);
	unsigned int value = rs_value + rt_value;

	set_register(instr.register_instruction.rd, value);
}

// SUB rd, rs, rt
void Cpu::sub(const instruction_union& instr)
{
	int rs_value = get_register(instr.register_instruction.rs);
	int rt_value = get_register(instr.register_instruction.rt);
	unsigned int value = rs_value - rt_value;
	
	if (rt_value > 0 && rs_value > 0) {
		if ((int)value < 0)
		{
			throw overflow_exception();
		}
	}
	else if (rt_value < 0 && rs_value < 0) {
		if ((int)value > 0)
		{
			throw overflow_exception();
		}
	}
	
	set_register(instr.register_instruction.rd, value);
}

// SUBU rd, rs, rt
void Cpu::sub_unsigned(const instruction_union& instr)
{
	int rs_value = get_register(instr.register_instruction.rs);
	int rt_value = get_register(instr.register_instruction.rt);
	unsigned int value = rs_value - rt_value;
	set_register(instr.register_instruction.rd, value);
}

// SLT rd, rs, rt
void Cpu::set_on_less_than(const instruction_union& instr)
{
	int rt_value = get_register(instr.register_instruction.rt);
	int rs_value = get_register(instr.register_instruction.rs);

	set_register(instr.register_instruction.rd, rs_value < rt_value ? 1 : 0);
}

// SLTU rd, rs, rt
void Cpu::set_on_less_than_unsigned(const instruction_union& instr)
{
	unsigned int rt_value = get_register(instr.register_instruction.rt);
	unsigned int rs_value = get_register(instr.register_instruction.rs);

	set_register(instr.register_instruction.rd, rs_value < rt_value ? 1 : 0);
}

// AND rd, rs, rt
void Cpu::and(const instruction_union& instr)
{
	unsigned int rs_value = get_register(instr.register_instruction.rs);
	unsigned int rt_value = get_register(instr.register_instruction.rt);
	unsigned int value = rs_value & rt_value;
	set_register(instr.register_instruction.rd, value);
}

// OR rd, rs, rt
void Cpu::or(const instruction_union& instr)
{
	unsigned int value = get_register(instr.register_instruction.rs) | get_register(instr.register_instruction.rt);
	set_register(instr.register_instruction.rd, value);
}

// XOR rd, rs, rt
void Cpu::xor (const instruction_union& instr)
{
	unsigned int value = get_register(instr.register_instruction.rs) ^ get_register(instr.register_instruction.rt);
	set_register(instr.register_instruction.rd, value);
}

// NOR rd, rs, rt
void Cpu::nor(const instruction_union& instr)
{
	unsigned int value = ~(get_register(instr.register_instruction.rs) | get_register(instr.register_instruction.rt));
	set_register(instr.register_instruction.rd, value);
}

// shift operations
// SLL rd, rt, shamt
void Cpu::shift_left_logical(const instruction_union& instr)
{
	unsigned int value = get_register(instr.register_instruction.rt) << instr.register_instruction.shamt;
	set_register(instr.register_instruction.rd, value);
}

// SRL rd, rt, shamt
void Cpu::shift_right_logical(const instruction_union& instr)
{
	unsigned int value = get_register(instr.register_instruction.rt) >> instr.register_instruction.shamt;
	set_register(instr.register_instruction.rd, value);
}

// SRA rd, rt, shamt
void Cpu::shift_right_arithmetic(const instruction_union& instr)
{
	unsigned int value = (int)(get_register(instr.register_instruction.rt)) >> instr.register_instruction.shamt;
	set_register(instr.register_instruction.rd, value);
}

// SLLV rd, rt, rs
void Cpu::shift_left_logical_variable(const instruction_union& instr)
{
	unsigned int value = get_register(instr.register_instruction.rs) << (0x1F & get_register(instr.register_instruction.rt));
	set_register(instr.register_instruction.rd, value);
}

// SRLV rd, rt, rs
void Cpu::shift_right_logical_variable(const instruction_union& instr)
{
	unsigned int value = get_register(instr.register_instruction.rs) >> (0x1F & get_register(instr.register_instruction.rt));
	set_register(instr.register_instruction.rd, value);
}

// SRAV rd, rt, rs
void Cpu::shift_right_arithmetic_variable(const instruction_union& instr)
{
	unsigned int value = (int)get_register(instr.register_instruction.rs) >> (0x1F & get_register(instr.register_instruction.rt));
	set_register(instr.register_instruction.rd, value);
}

// multiply/divide
// MULT rs, rt
void Cpu::mult(const instruction_union& instr)
{
	long long result = (int)(get_register(instr.register_instruction.rs)) * (int)(get_register(instr.register_instruction.rt));
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// MULTU rs, rt
void Cpu::mult_unsigned(const instruction_union& instr)
{
	unsigned long long result = get_register(instr.register_instruction.rs) * get_register(instr.register_instruction.rt);
	hi = result >> 32;
	lo = result & 0xFFFF;
}

// DIV rs, rt
void Cpu::div(const instruction_union& instr)
{
	hi = (int)get_register(instr.register_instruction.rs) / (int)get_register(instr.register_instruction.rt);
	lo = (int)get_register(instr.register_instruction.rs) % (int)get_register(instr.register_instruction.rt);
}

// DIVU rs, rt
void Cpu::div_unsigned(const instruction_union& instr)
{
	hi = get_register(instr.register_instruction.rs) / get_register(instr.register_instruction.rt);
	lo = get_register(instr.register_instruction.rs) % get_register(instr.register_instruction.rt);
}

// MFHI rd
void Cpu::move_from_hi(const instruction_union& instr)
{
	set_register(instr.register_instruction.rd, hi);
}

// MFLO rd
void Cpu::move_from_lo(const instruction_union& instr)
{
	set_register(instr.register_instruction.rd, lo);
}

// MTHI rd
void Cpu::move_to_hi(const instruction_union& instr)
{
	hi = get_register(instr.register_instruction.rs);
}

// MTLO rd
void Cpu::move_to_lo(const instruction_union& instr)
{
	lo = get_register(instr.register_instruction.rs);
}

// jump instructions
// J target
void Cpu::jump(const instruction_union& instr)
{
	unsigned int target = instr.jump_instruction.target << 2;

	pc = target | (0xF0000000 & pc);
}

// JAL target
void Cpu::jump_and_link(const instruction_union& instr)
{
	set_register(31, pc);
	jump(instr);
}

// JR rs
void Cpu::jump_register(const instruction_union& instr)
{
	pc = get_register(instr.register_instruction.rs);
}

// JALR rs, rd
void Cpu::jump_and_link_register(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// branch instructions
// BEQ rs, rt, offset
void Cpu::branch_on_equal(const instruction_union& instr)
{
	unsigned int rs_value = get_register(instr.immediate_instruction.rs);
	unsigned int rt_value = get_register(instr.immediate_instruction.rt);
	if (rs_value == rt_value)
	{
		unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
		pc += offset;
		pc -= 4;
	}
}

// BNE rs, rt, offset
void Cpu::branch_on_not_equal(const instruction_union& instr)
{
	if (get_register(instr.immediate_instruction.rs) != get_register(instr.immediate_instruction.rt))
	{
		unsigned int offset = (short)instr.immediate_instruction.immediate << 2;
		pc += offset;
		pc -= 4;
	}
}

// BLEZ rs, offset
void Cpu::branch_on_less_than_or_equal_zero(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// BGTZ rs, offset
void Cpu::branch_on_greater_than_zero(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// BLTZ rs, offset
void Cpu::branch_on_less_than_zero(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// BGEZ rs, offset
void Cpu::branch_on_greater_than_or_equal_zero(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// BLTZAL rs, offset
void Cpu::branch_on_less_than_zero_and_link(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// BGEZAL rs, offset
void Cpu::branch_on_greater_than_or_equal_zero_and_link(const instruction_union& instr)
{
	throw std::logic_error("not implemented");
}

// special instructions
// SYSCALL
void Cpu::system_call(const instruction_union& instr)
{
	throw sys_call();
}

// BREAK
void Cpu::breakpoint(const instruction_union& instr)
{
	throw breakpoint_exception();
}