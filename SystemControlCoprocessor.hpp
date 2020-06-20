#pragma once
#include "Coprocessor.hpp"
#include "Bus.hpp"
#include "SystemControlTypes.hpp"
#include <sstream>

class SystemControlCoprocessor : public Cop, public Bus::BusDevice {
public:

	static SystemControlCoprocessor * get_instance();

	virtual bool is_address_for_device(unsigned int address) final;

	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;
	virtual unsigned int get_word(unsigned int address) final;
	virtual void set_word(unsigned int address, unsigned int value) final;

	system_control::interrupt_register interrupt_status_register, interrupt_mask_register;

	void save_state(std::stringstream& file) override;
	void load_state(std::stringstream& file) override;

	void execute(const instruction_union& instruction) final;

	unsigned int get_control_register(system_control::register_names register_name);
	void set_control_register(system_control::register_names register_name, unsigned int value);

	void set_irq_bits(unsigned int irq_bits);
	void trigger_interrupts();
	void generate_interrupt(system_control::excode excode);

private:
	SystemControlCoprocessor();
	~SystemControlCoprocessor();

	// I_STAT_SIZE and I_MASK_SIZE only use the first 2 bytes
	// and the next 2 in both are considered garbage areas
	static const unsigned int I_STAT_START = 0x1F801070;
	static const unsigned int I_STAT_END = I_STAT_START + 4;
	static const unsigned int I_MASK_START = 0x1F801074;
	static const unsigned int I_MASK_END = I_MASK_START + 4;

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