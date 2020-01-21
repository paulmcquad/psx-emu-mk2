#include <memory>
#include <map>

class Coprocessor;
class Ram;

struct immediate_instruction;
struct register_instruction;
struct jump_instruction;

class Cpu : public std::enable_shared_from_this<Cpu>
{
private:
	std::map<unsigned int, std::shared_ptr<Coprocessor>> coprocessors;
	std::shared_ptr<Ram> ram = nullptr;

public:

	void init(std::shared_ptr<Ram> _ram);
	void tick();
	void execute(unsigned int instruction);
	void execute_special(unsigned int instruction);
	void execute_bcond(unsigned int instruction);
	void execute_cop(unsigned int instruction);

	unsigned int get_register(int index);
	void set_register(int index, unsigned int value);

	unsigned int get_immediate_base_addr(const immediate_instruction& instr);

	unsigned int gp_registers[32] = { 0 };
	unsigned int cp0_registers[16] = { 0 };
	unsigned int cp2_data_registers[32] = { 0 };
	unsigned int cp2_control_registers[32] = { 0 };

	unsigned int hi = 0;
	unsigned int lo = 0;
	unsigned int pc = 0;

	unsigned int next_instruction = 0x0;

	// load/store
	void load_byte(const immediate_instruction& instr);
	void load_byte_unsigned(const immediate_instruction& instr);
	void load_halfword(const immediate_instruction& instr);
	void load_halfword_unsigned(const immediate_instruction& instr);
	void load_word(const immediate_instruction& instr);
	void load_word_left(const immediate_instruction& instr);
	void load_word_right(const immediate_instruction& instr);

	void store_byte(const immediate_instruction& instr);
	void store_halfword(const immediate_instruction& instr);
	void store_word(const immediate_instruction& instr);
	void store_word_left(const immediate_instruction& instr);
	void store_word_right(const immediate_instruction& instr);

	// ALU immediate operations
	void add_immediate(const immediate_instruction& instr);
	void add_immediate_unsigned(const immediate_instruction& instr);
	void set_on_less_than_immediate(const immediate_instruction& instr);
	void set_on_less_than_unsigned_immediate(const immediate_instruction& instr);
	void and_immediate(const immediate_instruction& instr);
	void or_immediate(const immediate_instruction& instr);
	void xor_immediate(const immediate_instruction& instr);
	void load_upper_immediate(const immediate_instruction& instr);

	// three operand register type
	void add(const register_instruction& instr);
	void add_unsigned(const register_instruction& instr);
	void sub(const register_instruction& instr);
	void sub_unsigned(const register_instruction& instr);
	void set_on_less_than(const register_instruction& instr);
	void set_on_less_than_unsigned(const register_instruction& instr);
	void and(const register_instruction& instr);
	void or (const register_instruction& instr);
	void xor(const register_instruction& instr);
	void nor(const register_instruction& instr);

	// shift operations
	void shift_left_logical(const register_instruction& instr);
	void shift_right_logical(const register_instruction& instr);
	void shift_right_arithmetic(const register_instruction& instr);
	void shift_left_logical_variable(const register_instruction& instr);
	void shift_right_logical_variable(const register_instruction& instr);
	void shift_right_arithmetic_variable(const register_instruction& instr);

	// multiply/divide
	void mult(const register_instruction& instr);
	void mult_unsigned(const register_instruction& instr);
	void div(const register_instruction& instr);
	void div_unsigned(const register_instruction& instr);
	void move_from_hi(const register_instruction& instr);
	void move_from_lo(const register_instruction& instr);
	void move_to_hi(const register_instruction& instr);
	void move_to_lo(const register_instruction& instr);

	// jump instructions
	void jump(const jump_instruction& instr);
	void jump_and_link(const jump_instruction& instr);
	void jump_register(const register_instruction& instr);
	void jump_and_link_register(const register_instruction& instr);

	// branch instructions
	void branch_on_equal(const immediate_instruction& instr);
	void branch_on_not_equal(const immediate_instruction& instr);
	void branch_on_less_than_or_equal_zero(const immediate_instruction& instr);
	void branch_on_greater_than_zero(const immediate_instruction& instr);
	void branch_on_less_than_zero(const immediate_instruction& instr);
	void branch_on_greater_than_or_equal_zero(const immediate_instruction& instr);
	void branch_on_less_than_zero_and_link(const immediate_instruction& instr);
	void branch_on_greater_than_or_equal_zero_and_link(const immediate_instruction& instr);

	// special instructions
	void system_call();
	void breakpoint();

	// co-processor instructions
	void load_word_to_cop(const immediate_instruction& instr);
	void store_word_from_cop(const immediate_instruction& instr);
	void move_to_cop(const register_instruction& instr);
	void move_from_cop(const register_instruction& instr);
	void move_control_to_cop(const register_instruction& instr);
	void move_control_from_cop(const register_instruction& instr);
	void move_control_to_cop_fun(const register_instruction& instr);

	// system control
	void move_to_cp0(const register_instruction& instr);
	void move_from_cp0(const register_instruction& instr);
	void read_indexed_tlb();
	void write_indexed_tlb();
	void write_random_tlb();
	void probe_tlb_for_matching_entry();
	void restore_from_exception();
};