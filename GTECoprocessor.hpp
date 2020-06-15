#include "Coprocessor.hpp"

class GTECoprocessor : public Cop {
public:
	static GTECoprocessor * get_instance();

	void save_state(std::stringstream& file) override;
	void load_state(std::stringstream& file) override;

	void execute(const instruction_union& instruction) final;

private:
	GTECoprocessor() = default;
	~GTECoprocessor() = default;

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