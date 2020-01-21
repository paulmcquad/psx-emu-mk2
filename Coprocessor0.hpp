#include "Coprocessor.hpp"

class Ram;
class Cpu;

class Coprocessor0 : public Coprocessor {
public:
	Coprocessor0(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu);

	void execute(unsigned int instruction) final;

private:
	unsigned int get_control_register(unsigned int index);
	void set_control_register(unsigned int index, unsigned int value);

	void load_word_to_cop(const immediate_instruction& instr) final;
	void store_word_from_cop(const immediate_instruction& instr) final;
	void move_to_cop(const register_instruction& instr) final;
	void move_from_cop(const register_instruction& instr) final;
	void move_control_to_cop(const register_instruction& instr) final;
	void move_control_from_cop(const register_instruction& instr) final;
	void move_control_to_cop_fun(const register_instruction& instr) final;

	void move_to_cp0(const register_instruction& instr);
	void move_from_cp0(const register_instruction& instr);
	void read_indexed_tlb_entry(const register_instruction& instr);
	void write_indexed_tlb_entry(const register_instruction& instr);
	void write_random_tlb_entry(const register_instruction& instr);
	void probe_tlb_for_matching_entry(const register_instruction& instr);
	void restore_from_exception(const register_instruction& instr);
	
	unsigned int control_registers[32] = { 0 };
};