#include <memory>
#include <map>

class Coprocessor;
class Ram;

struct ImmediateInstruction;
struct RegisterInstruction;
struct JumpInstruction;

class Cpu : public std::enable_shared_from_this<Cpu>
{
private:
	std::map<unsigned int, std::shared_ptr<Coprocessor>> coprocessors;
	std::shared_ptr<Ram> ram = nullptr;

public:

	void init(std::shared_ptr<Ram> _ram);
	void reset();
	void run_cycle();
	void execute(unsigned int instruction);
	void execute_special(unsigned int instruction);
	void execute_bcond(unsigned int instruction);

	unsigned int get_register(int index);
	void set_register(int index, unsigned int value);

	unsigned int get_immediate_base_addr(const ImmediateInstruction& instr);

	unsigned int gp_registers[32] = { 0 };
	unsigned int cp0_registers[16] = { 0 };
	unsigned int cp2_data_registers[32] = { 0 };
	unsigned int cp2_control_registers[32] = { 0 };

	unsigned int hi = 0;
	unsigned int lo = 0;
	unsigned int pc = 0;

	unsigned int next_instruction = 0x0;

	// load/store
	void load_byte(const ImmediateInstruction& instr);
	void load_byte_unsigned(const ImmediateInstruction& instr);
	void load_halfword(const ImmediateInstruction& instr);
	void load_halfword_unsigned(const ImmediateInstruction& instr);
	void load_word(const ImmediateInstruction& instr);
	void load_word_left(const ImmediateInstruction& instr);
	void load_word_right(const ImmediateInstruction& instr);

	void store_byte(const ImmediateInstruction& instr);
	void store_halfword(const ImmediateInstruction& instr);
	void store_word(const ImmediateInstruction& instr);
	void store_word_left(const ImmediateInstruction& instr);
	void store_word_right(const ImmediateInstruction& instr);

	// ALU immediate operations
	void add_immediate(const ImmediateInstruction& instr);
	void add_immediate_unsigned(const ImmediateInstruction& instr);
	void set_on_less_than_immediate(const ImmediateInstruction& instr);
	void set_on_less_than_unsigned_immediate(const ImmediateInstruction& instr);
	void and_immediate(const ImmediateInstruction& instr);
	void or_immediate(const ImmediateInstruction& instr);
	void xor_immediate(const ImmediateInstruction& instr);
	void load_upper_immediate(const ImmediateInstruction& instr);

	// three operand register type
	void add(const RegisterInstruction& instr);
	void add_unsigned(const RegisterInstruction& instr);
	void sub(const RegisterInstruction& instr);
	void sub_unsigned(const RegisterInstruction& instr);
	void set_on_less_than(const RegisterInstruction& instr);
	void set_on_less_than_unsigned(const RegisterInstruction& instr);
	void and(const RegisterInstruction& instr);
	void or (const RegisterInstruction& instr);
	void xor(const RegisterInstruction& instr);
	void nor(const RegisterInstruction& instr);

	// shift operations
	void shift_left_logical(const RegisterInstruction& instr);
	void shift_right_logical(const RegisterInstruction& instr);
	void shift_right_arithmetic(const RegisterInstruction& instr);
	void shift_left_logical_variable(const RegisterInstruction& instr);
	void shift_right_logical_variable(const RegisterInstruction& instr);
	void shift_right_arithmetic_variable(const RegisterInstruction& instr);

	// multiply/divide
	void mult(const RegisterInstruction& instr);
	void mult_unsigned(const RegisterInstruction& instr);
	void div(const RegisterInstruction& instr);
	void div_unsigned(const RegisterInstruction& instr);
	void move_from_hi(const RegisterInstruction& instr);
	void move_from_lo(const RegisterInstruction& instr);
	void move_to_hi(const RegisterInstruction& instr);
	void move_to_lo(const RegisterInstruction& instr);

	// jump instructions
	void jump(const JumpInstruction& instr);
	void jump_and_link(const JumpInstruction& instr);
	void jump_register(const RegisterInstruction& instr);
	void jump_and_link_register(const RegisterInstruction& instr);

	// branch instructions
	void branch_on_equal(const ImmediateInstruction& instr);
	void branch_on_not_equal(const ImmediateInstruction& instr);
	void branch_on_less_than_or_equal_zero(const ImmediateInstruction& instr);
	void branch_on_greater_than_zero(const ImmediateInstruction& instr);
	void branch_on_less_than_zero(const ImmediateInstruction& instr);
	void branch_on_greater_than_or_equal_zero(const ImmediateInstruction& instr);
	void branch_on_less_than_zero_and_link(const ImmediateInstruction& instr);
	void branch_on_greater_than_or_equal_zero_and_link(const ImmediateInstruction& instr);

	// special instructions
	void system_call();
	void breakpoint();

	// co-processor instructions
	void load_word_to_cop(const ImmediateInstruction& instr);
	void store_word_from_cop(const ImmediateInstruction& instr);
	void move_to_cop(const RegisterInstruction& instr);
	void move_from_cop(const RegisterInstruction& instr);
	void move_control_to_cop(const RegisterInstruction& instr);
	void move_control_from_cop(const RegisterInstruction& instr);
	void move_control_to_cop_fun(const RegisterInstruction& instr);

	// system control
	void move_to_cp0(const RegisterInstruction& instr);
	void move_from_cp0(const RegisterInstruction& instr);
	void read_indexed_tlb();
	void write_indexed_tlb();
	void write_random_tlb();
	void probe_tlb_for_matching_entry();
	void restore_from_exception();
};