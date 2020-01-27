#include "Coprocessor.hpp"

class Ram;
class Cpu;

class Coprocessor0 : public Coprocessor {
public:
	enum class register_names : unsigned int
	{
		BPC = 3,
		BDA = 5,
		JUMPDEST = 6,
		DCIC = 7,
		BADVADDR = 8,
		BDAM = 9,
		BPCM = 11,
		SR = 12,
		CAUSE = 13,
		EPC = 14,
		PRID = 15
	};

	Coprocessor0(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu);

	void execute(const instruction_union& instruction) final;

	unsigned int get_control_register(register_names register_name);

private:
	unsigned int get_control_register(unsigned int index);
	void set_control_register(unsigned int index, unsigned int value);
	void load_word_to_cop(const instruction_union& instr) final;
	void store_word_from_cop(const instruction_union& instr) final;
	void move_to_cop(const instruction_union& instr) final;
	void move_from_cop(const instruction_union& instr) final;
	void move_control_to_cop(const instruction_union& instr) final;
	void move_control_from_cop(const instruction_union& instr) final;
	void move_control_to_cop_fun(const instruction_union& instr) final;

	void move_to_cp0(const instruction_union& instr);
	void move_from_cp0(const instruction_union& instr);
	void read_indexed_tlb_entry(const instruction_union& instr);
	void write_indexed_tlb_entry(const instruction_union& instr);
	void write_random_tlb_entry(const instruction_union& instr);
	void probe_tlb_for_matching_entry(const instruction_union& instr);
	void restore_from_exception(const instruction_union& instr);
	
	unsigned int control_registers[32] = { 0 };
};