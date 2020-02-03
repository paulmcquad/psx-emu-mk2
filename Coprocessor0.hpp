#include "Coprocessor.hpp"

class Ram;
class Cpu;

class Cop0 : public Cop {
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

	union status_register
	{
		unsigned int raw;
		struct
		{
			// current interrupt enable
			unsigned int IEc : 1;
			// current kernal/user mode
			unsigned int KUc : 1;
			// previous interrupt disable
			unsigned int IEp : 1;
			// previouse kernal/user mode
			unsigned int KUp : 1;
			// old interrupt disable
			unsigned int IEo : 1;
			// old kernal/user mode
			unsigned int KUo : 1;
			// not used
			unsigned int NA0 : 2;
			// interrupt mask
			unsigned int Im : 8;
			// isolate cache
			unsigned int Isc : 1;
			// swapped cache mode (not used by psx)
			unsigned int Swc : 1;
			// set cahce parity bits 0
			unsigned int PZ : 1;
			// result of last load operation with the D-cache isolated
			unsigned int CM : 1;
			// cache parity error
			unsigned int PE : 1;
			// TLB shutdown
			unsigned int TS : 1;
			// boot exception vector
			unsigned int BEV : 1;
			// not used
			unsigned int NA1 : 2;
			// reverse endianness (not used by psx)
			unsigned int RE : 1;
			// not used
			unsigned int NA2 : 2;
			// COP0 enable
			unsigned int CU0 : 1;
			// COP1 enable (not used)
			unsigned int CU1 : 1;
			// COP2 enable
			unsigned int CU2 : 1;
			// COP3 enable (not used)
			unsigned int CU3 : 1;
		};
	};

	Cop0(std::shared_ptr<Ram> _ram, std::shared_ptr<Cpu> _cpu);

	void execute(const instruction_union& instruction) final;

	unsigned int get_control_register(register_names register_name);
	void set_control_register(register_names register_name, unsigned int value);

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
	void restore_from_exception(const instruction_union& instr);
	
	unsigned int control_registers[32] = { 0 };
};