#include "Coprocessor.hpp"

class Coprocessor2 : public Coprocessor {
public:
	Coprocessor2(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu);

	void execute(unsigned int instruction) final;

private:
	unsigned int get_data_register(unsigned int index);
	void set_data_register(unsigned int index, unsigned int value);

	unsigned int get_control_register(unsigned int index);
	void set_control_register(unsigned int index, unsigned int value);

	void load_word_to_cop(const ImmediateInstruction& instr) final;
	void store_word_from_cop(const ImmediateInstruction& instr) final;
	void move_to_cop(const RegisterInstruction& instr) final;
	void move_from_cop(const RegisterInstruction& instr) final;
	void move_control_to_cop(const RegisterInstruction& instr) final;
	void move_control_from_cop(const RegisterInstruction& instr) final;
	void move_control_to_cop_fun(const RegisterInstruction& instr) final;

	unsigned int data_registers[32] = { 0 };
	unsigned int control_registers[32] = { 0 };
};