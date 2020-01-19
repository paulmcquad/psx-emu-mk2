#include "Coprocessor.hpp"

class Coprocessor2 : public Coprocessor {
public:
	Coprocessor2(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu);

	void load_word_to_cop(const ImmediateInstruction& instr) final;
	void store_word_from_cop(const ImmediateInstruction& instr) final;
	void move_to_cop(const RegisterInstruction& instr) final;
	void move_from_cop(const RegisterInstruction& instr) final;
	void move_control_to_cop(const RegisterInstruction& instr) final;
	void move_control_from_cop(const RegisterInstruction& instr) final;
	void move_control_to_cop_fun(const RegisterInstruction& instr) final;
};