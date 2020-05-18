#pragma once

#include "Coprocessor.hpp"
#include "Bus.hpp"

class Ram;
class Cpu;

class SystemControlCoprocessor : public Cop, public Bus::BusDevice {
public:

	class SystemControlInterface
	{
	public:
		virtual void trigger_pending_interrupts(SystemControlCoprocessor* system_control_processor) = 0;
	};

	bool is_address_for_device(unsigned int address) final;

	unsigned char get_byte(unsigned int address) final;
	void set_byte(unsigned int address, unsigned char value) final;

	unsigned int get_word(unsigned int address) final;
	void set_word(unsigned int address, unsigned int value) final;

	union
	{
		unsigned int value;
		unsigned char bytes[4];
		struct
		{
			unsigned int IRQ_BITS : 11;
			unsigned int NA : 21;
		};
		struct
		{
			unsigned int IRQ0_VBLANK : 1;
			unsigned int IRQ1_GPU : 1;
			unsigned int IRQ2_CDROM : 1;
			unsigned int IRQ3_DMA : 1;
			unsigned int IRQ4_TMR0 : 1;
			unsigned int IRQ5_TMR1 : 1;
			unsigned int IRQ6_TMR2 : 1;
			unsigned int IRQ7_CTRL_MEM_CRD : 1;
			unsigned int IRQ8_SIO : 1;
			unsigned int IRQ9_SPU : 1;
			unsigned int IRQ10_LIGHTPEN : 1;
			unsigned int NA : 21;
		};
	} interrupt_status_register, interrupt_mask_register;

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

	// convenience getter/setters as I was getting sick
	// of getting unsigned int from the registers, setting the union struct
	// altering it and then setting the register again with the struct.raw member variable
	template <class T>
	T get()
	{
		throw std::logic_error("unsupported");
	}

	template <class T>
	void set(T value)
	{
		throw std::logic_error("unsupported");
	}

	// status register - get and set
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
			// set cache parity bits 0
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

	template <>
	status_register get<status_register>()
	{
		status_register result;
		result.raw = get_control_register(register_names::SR);
		return result;
	}

	template<>
	void set<status_register>(status_register value)
	{
		set_control_register(register_names::SR, value.raw);
	}

	// cause register - get and set
	union cause_register
	{
		unsigned int raw;
		struct
		{
			// not used
			unsigned int NA0 : 2;
			// what kind of exception occured
			unsigned int Excode : 5;
			// not used
			unsigned int NA1 : 1;
			// interrupt pending
			unsigned int Ip : 8;
			// not used
			unsigned int NA2 : 12;
			// coprocessor number if exception occurred in it
			unsigned int CE : 2;
			// not used
			unsigned int NA3 : 1;
			// last exception points to exception
			unsigned int BD : 1;
		};
	};

	template <>
	cause_register get<cause_register>()
	{
		cause_register result;
		result.raw = get_control_register(register_names::CAUSE);
		return result;
	}

	template<>
	void set<cause_register>(cause_register value)
	{
		set_control_register(register_names::CAUSE, value.raw);
	}

	enum class excode : unsigned int
	{
		// interrupt
		INT = 0x00,
		// not used
		MOD = 0x01,
		TLBL = 0x02,
		TLBS = 0x03,
		// address error, data load or instruction fetch
		AdEL = 0x04,
		// address error, data store, outside KUseg in user mode or
		// misaligned address
		AdES = 0x05,
		// Bus error on instruction fetch
		IBE = 0x06,
		// Bus error on data load/store
		DBE = 0x07,
		// syscall
		Syscall = 0x08,
		// breakpoint
		BP = 0x09,
		// reserved instruction
		RI = 0x0A,
		// coprocessor unusable
		CpU = 0x0B,
		// arithmetic overflow
		Ov = 0x0C
	};

	enum class exception_vector : unsigned int
	{
		RESET = 0xBFC00000,
		COP0_BREAK_BEV0 = 0x80000040,
		COP0_BREAK_BEV1 = 0xBFC00140,
		GENERAL_BEV0 = 0x80000080,
		GENERAL_BEV1 = 0xBFC00180,
		// not used
		UTLB_MISS_BEV0 = 0x80000000,
		UTLB_MISS_BEV1 = 0xBFC00100
	};

	SystemControlCoprocessor(std::shared_ptr<Bus> _bus, std::shared_ptr<Cpu> _cpu);

	void save_state(std::ofstream& file) override;
	void load_state(std::ifstream& file) override;

	void execute(const instruction_union& instruction) final;

	unsigned int get_control_register(register_names register_name);
	void set_control_register(register_names register_name, unsigned int value);

	void trigger_pending_interrupts();

	void register_system_control_device(SystemControlInterface * interface)
	{
		system_control_devices[num_system_control_devices] = interface;
		num_system_control_devices++;
	}

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

	unsigned int num_system_control_devices = 0;
	SystemControlInterface * system_control_devices[7] = { nullptr };
};