#include "Coprocessor.hpp"

class Cop2 : public Cop {
public:
	Cop2(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu);

	void save_state(std::ofstream& file) override;
	void load_state(std::ifstream& file) override;

	void execute(const instruction_union& instruction) final;

private:
	unsigned int get_data_register(unsigned int index);
	void set_data_register(unsigned int index, unsigned int value);

	unsigned int get_control_register(unsigned int index);
	void set_control_register(unsigned int index, unsigned int value);

	void load_word_to_cop(const instruction_union& instr) final;
	void store_word_from_cop(const instruction_union& instr) final;
	void move_to_cop(const instruction_union& instr) final;
	void move_from_cop(const instruction_union& instr) final;
	void move_control_to_cop(const instruction_union& instr) final;
	void move_control_from_cop(const instruction_union& instr) final;
	void move_control_to_cop_fun(const instruction_union& instr) final;

	unsigned int data_registers[32] = { 0 };
	unsigned int control_registers[32] = { 0 };
};