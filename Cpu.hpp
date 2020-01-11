#include <string>

struct ImmediateInstruction
{
	unsigned int op : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int immediate : 16;
};

struct JumpInstruction
{
	unsigned int op : 6;
	unsigned int target : 26;
};

struct RegisterInstruction
{
	unsigned int op : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int rd : 5;
	unsigned int shamt : 5;
	unsigned int funct : 6;
};

struct CopInstruction
{
	unsigned int op : 6;
	unsigned int 
};

class Cpu
{
public:
	void reset();
	void run();

private:
	static const int num_gp_registers = 32;
	static const int num_cp0_registers = 16;

	int gp_registers[num_gp_registers] = { 0 };
	int cp0_registers[num_cp0_registers] = { 0 };

	int hi = 0;
	int lo = 0;
	int pc = 0;

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
	void set_on_less_than_immediate(const RegisterInstruction& instr);
	void and(const RegisterInstruction& instr);
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