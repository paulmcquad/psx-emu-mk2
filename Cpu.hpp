#include <memory>
#include <map>
#include <unordered_map>
#include "InstructionTypes.hpp"

class Cop0;
class Cop2;
class Ram;

enum class cpu_instructions : unsigned char;
enum class cpu_special_funcs : unsigned char;
enum class cpu_bconds : unsigned char;

// ref:
// https://svkt.org/~simias/guide.pdf
// https://problemkaputt.de/psx-spx.htm
// http://hitmen.c02.at/files/docs/psx/psx.pdf
class Cpu : public std::enable_shared_from_this<Cpu>
{
public:
	std::shared_ptr<Cop0> cop0 = nullptr;
	std::shared_ptr<Cop2> cop2 = nullptr;

	std::shared_ptr<Ram> ram = nullptr;

	std::unordered_map<cpu_instructions, void (Cpu::*)(const instruction_union&)> main_instructions;
	std::unordered_map<cpu_special_funcs, void (Cpu::*)(const instruction_union&)> special_instructions;
	std::unordered_map<cpu_bconds, void (Cpu::*)(const instruction_union&)> bcond_instructions;

	// 3 stage register file to simulate the load delay
	// shadow first -> shadow second -> gp_registers
	struct
	{
		unsigned int gp_registers[32] = { 0 };
		unsigned int shadow_gp_registers_last[32] = { 0 };
		unsigned int shadow_gp_registers_first[32] = { 0 };
		void merge()
		{
			memcpy(&gp_registers, &shadow_gp_registers_last, sizeof(unsigned int) * 32);
			memcpy(&shadow_gp_registers_last, &shadow_gp_registers_first, sizeof(unsigned int) * 32);
		}
		void reset()
		{
			for (int index = 0; index < 32; index++)
			{
				gp_registers[index] = shadow_gp_registers_first[index] = shadow_gp_registers_last[index] = 0;
			}
		}
	} register_file;

	Cpu();
	void init(std::shared_ptr<Ram> _ram);
	void reset();
	void tick();

	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	void execute(unsigned int instruction);
	void execute_special(const instruction_union& instr);
	void execute_bcond(const instruction_union& instr);
	void execute_cop(const instruction_union& instr);

	unsigned int get_register(int index);
	void set_register(int index, unsigned int value, bool delay = false);

	unsigned int get_immediate_base_addr(const instruction_union& instr);

	unsigned int hi = 0;
	unsigned int lo = 0;

	unsigned int current_pc = 0;
	unsigned int next_pc = 0;

	unsigned int current_instruction = 0;
	unsigned int next_instruction = 0;

	bool in_delay_slot = false;

	// load/store
	inline void load_byte(const instruction_union& instr);
	inline void load_byte_unsigned(const instruction_union& instr);
	inline void load_halfword(const instruction_union& instr);
	inline void load_halfword_unsigned(const instruction_union& instr);
	inline void load_word(const instruction_union& instr);
	inline void load_word_left(const instruction_union& instr);
	inline void load_word_right(const instruction_union& instr);

	inline void store_byte(const instruction_union& instr);
	inline void store_halfword(const instruction_union& instr);
	inline void store_word(const instruction_union& instr);
	inline void store_word_left(const instruction_union& instr);
	inline void store_word_right(const instruction_union& instr);

	// ALU immediate operations
	inline void add_immediate(const instruction_union& instr);
	inline void add_immediate_unsigned(const instruction_union& instr);
	inline void set_on_less_than_immediate(const instruction_union& instr);
	inline void set_on_less_than_unsigned_immediate(const instruction_union& instr);
	inline void and_immediate(const instruction_union& instr);
	inline void or_immediate(const instruction_union& instr);
	inline void xor_immediate(const instruction_union& instr);
	inline void load_upper_immediate(const instruction_union& instr);

	// three operand register type
	inline void add(const instruction_union& instr);
	inline void add_unsigned(const instruction_union& instr);
	inline void sub(const instruction_union& instr);
	inline void sub_unsigned(const instruction_union& instr);
	inline void set_on_less_than(const instruction_union& instr);
	inline void set_on_less_than_unsigned(const instruction_union& instr);
	inline void and(const instruction_union& instr);
	inline void or(const instruction_union& instr);
	inline void xor(const instruction_union& instr);
	inline void nor(const instruction_union& instr);

	// shift operations
	inline void shift_left_logical(const instruction_union& instr);
	inline void shift_right_logical(const instruction_union& instr);
	inline void shift_right_arithmetic(const instruction_union& instr);
	inline void shift_left_logical_variable(const instruction_union& instr);
	inline void shift_right_logical_variable(const instruction_union& instr);
	inline void shift_right_arithmetic_variable(const instruction_union& instr);

	// multiply/divide
	inline void mult(const instruction_union& instr);
	inline void mult_unsigned(const instruction_union& instr);
	inline void div(const instruction_union& instr);
	inline void div_unsigned(const instruction_union& instr);
	inline void move_from_hi(const instruction_union& instr);
	inline void move_from_lo(const instruction_union& instr);
	inline void move_to_hi(const instruction_union& instr);
	inline void move_to_lo(const instruction_union& instr);

	// jump instructions
	inline void jump(const instruction_union& instr);
	inline void jump_and_link(const instruction_union& instr);
	inline void jump_register(const instruction_union& instr);
	inline void jump_and_link_register(const instruction_union& instr);

	// branch instructions
	inline void branch_on_equal(const instruction_union& instr);
	inline void branch_on_not_equal(const instruction_union& instr);
	inline void branch_on_less_than_or_equal_zero(const instruction_union& instr);
	inline void branch_on_greater_than_zero(const instruction_union& instr);
	inline void branch_on_less_than_zero(const instruction_union& instr);
	inline void branch_on_greater_than_or_equal_zero(const instruction_union& instr);
	inline void branch_on_less_than_zero_and_link(const instruction_union& instr);
	inline void branch_on_greater_than_or_equal_zero_and_link(const instruction_union& instr);

	// special instructions
	inline void system_call(const instruction_union& instr);
	inline void breakpoint(const instruction_union& instr);
};