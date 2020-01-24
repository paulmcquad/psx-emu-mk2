#include <memory>
#include <map>
#include "InstructionTypes.hpp"

class Coprocessor;
class Ram;

enum class cpu_instructions : unsigned char;
enum class cpu_special_funcs : unsigned char;
enum class cpu_bconds : unsigned char;

class Cpu : public std::enable_shared_from_this<Cpu>
{
private:
	std::map<unsigned int, std::shared_ptr<Coprocessor>> coprocessors;
	std::shared_ptr<Ram> ram = nullptr;

	std::map<cpu_instructions, void (Cpu::*)(const instruction_union&)> main_instructions;
	std::map<cpu_special_funcs, void (Cpu::*)(const instruction_union&)> special_instructions;
	std::map<cpu_bconds, void (Cpu::*)(const instruction_union&)> bcond_instructions;

public:
	Cpu();

	void init(std::shared_ptr<Ram> _ram);
	void tick();
	void execute(unsigned int instruction);
	void execute_special(const instruction_union& instr);
	void execute_bcond(const instruction_union& instr);
	void execute_cop(const instruction_union& instr);

	unsigned int get_register(int index);
	void set_register(int index, unsigned int value);

	unsigned int get_immediate_base_addr(const instruction_union& instr);

	unsigned int gp_registers[32] = { 0 };
	unsigned int cp0_registers[16] = { 0 };
	unsigned int cp2_data_registers[32] = { 0 };
	unsigned int cp2_control_registers[32] = { 0 };

	unsigned int hi = 0;
	unsigned int lo = 0;
	unsigned int pc = 0;

	unsigned int next_instruction = 0x0;

	// load/store
	void load_byte(const instruction_union& instr);
	void load_byte_unsigned(const instruction_union& instr);
	void load_halfword(const instruction_union& instr);
	void load_halfword_unsigned(const instruction_union& instr);
	void load_word(const instruction_union& instr);
	void load_word_left(const instruction_union& instr);
	void load_word_right(const instruction_union& instr);

	void store_byte(const instruction_union& instr);
	void store_halfword(const instruction_union& instr);
	void store_word(const instruction_union& instr);
	void store_word_left(const instruction_union& instr);
	void store_word_right(const instruction_union& instr);

	// ALU immediate operations
	void add_immediate(const instruction_union& instr);
	void add_immediate_unsigned(const instruction_union& instr);
	void set_on_less_than_immediate(const instruction_union& instr);
	void set_on_less_than_unsigned_immediate(const instruction_union& instr);
	void and_immediate(const instruction_union& instr);
	void or_immediate(const instruction_union& instr);
	void xor_immediate(const instruction_union& instr);
	void load_upper_immediate(const instruction_union& instr);

	// three operand register type
	void add(const instruction_union& instr);
	void add_unsigned(const instruction_union& instr);
	void sub(const instruction_union& instr);
	void sub_unsigned(const instruction_union& instr);
	void set_on_less_than(const instruction_union& instr);
	void set_on_less_than_unsigned(const instruction_union& instr);
	void and(const instruction_union& instr);
	void or (const instruction_union& instr);
	void xor(const instruction_union& instr);
	void nor(const instruction_union& instr);

	// shift operations
	void shift_left_logical(const instruction_union& instr);
	void shift_right_logical(const instruction_union& instr);
	void shift_right_arithmetic(const instruction_union& instr);
	void shift_left_logical_variable(const instruction_union& instr);
	void shift_right_logical_variable(const instruction_union& instr);
	void shift_right_arithmetic_variable(const instruction_union& instr);

	// multiply/divide
	void mult(const instruction_union& instr);
	void mult_unsigned(const instruction_union& instr);
	void div(const instruction_union& instr);
	void div_unsigned(const instruction_union& instr);
	void move_from_hi(const instruction_union& instr);
	void move_from_lo(const instruction_union& instr);
	void move_to_hi(const instruction_union& instr);
	void move_to_lo(const instruction_union& instr);

	// jump instructions
	void jump(const instruction_union& instr);
	void jump_and_link(const instruction_union& instr);
	void jump_register(const instruction_union& instr);
	void jump_and_link_register(const instruction_union& instr);

	// branch instructions
	void branch_on_equal(const instruction_union& instr);
	void branch_on_not_equal(const instruction_union& instr);
	void branch_on_less_than_or_equal_zero(const instruction_union& instr);
	void branch_on_greater_than_zero(const instruction_union& instr);
	void branch_on_less_than_zero(const instruction_union& instr);
	void branch_on_greater_than_or_equal_zero(const instruction_union& instr);
	void branch_on_less_than_zero_and_link(const instruction_union& instr);
	void branch_on_greater_than_or_equal_zero_and_link(const instruction_union& instr);

	// special instructions
	void system_call(const instruction_union& instr);
	void breakpoint(const instruction_union& instr);

	// co-processor instructions
	void load_word_to_cop(const instruction_union& instr);
	void store_word_from_cop(const instruction_union& instr);
	void move_to_cop(const instruction_union& instr);
	void move_from_cop(const instruction_union& instr);
	void move_control_to_cop(const instruction_union& instr);
	void move_control_from_cop(const instruction_union& instr);
	void move_control_to_cop_fun(const instruction_union& instr);

	// system control
	void move_to_cp0(const instruction_union& instr);
	void move_from_cp0(const instruction_union& instr);
	void read_indexed_tlb(const instruction_union& instr);
	void write_indexed_tlb(const instruction_union& instr);
	void write_random_tlb(const instruction_union& instr);
	void probe_tlb_for_matching_entry(const instruction_union& instr);
	void restore_from_exception(const instruction_union& instr);
};