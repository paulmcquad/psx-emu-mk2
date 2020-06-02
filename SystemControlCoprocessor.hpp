#pragma once

#include "Coprocessor.hpp"
#include "Bus.hpp"
#include "SystemControlTypes.hpp"
#include <sstream>

class Ram;
class Cpu;

class SystemControlCoprocessor : public Cop, public Bus::BusDevice {
public:

	class SystemControlInterface
	{
	public:
		virtual bool trigger_pending_interrupts(SystemControlCoprocessor* system_control_processor, unsigned int & excode) = 0;
	};

	virtual bool is_peripheral() final { return true;  }

	virtual bus_device_type get_bus_device_type() final { return bus_device_type::INTERRUPT_CONTROL; }

	virtual bool is_address_for_device(unsigned int address) final;

	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;

	system_control::interrupt_register interrupt_status_register, interrupt_mask_register;

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
	template <>
	system_control::status_register get<system_control::status_register>()
	{
		system_control::status_register result;
		result.raw = get_control_register(system_control::register_names::SR);
		return result;
	}

	template<>
	void set<system_control::status_register>(system_control::status_register value)
	{
		set_control_register(system_control::register_names::SR, value.raw);
	}

	// cause register - get and set
	template <>
	system_control::cause_register get<system_control::cause_register>()
	{
		system_control::cause_register result;
		result.raw = get_control_register(system_control::register_names::CAUSE);
		return result;
	}

	template<>
	void set<system_control::cause_register>(system_control::cause_register value)
	{
		set_control_register(system_control::register_names::CAUSE, value.raw);
	}

	SystemControlCoprocessor(std::shared_ptr<Bus> _bus, std::shared_ptr<Cpu> _cpu);

	void save_state(std::stringstream& file) override;
	void load_state(std::stringstream& file) override;

	void execute(const instruction_union& instruction) final;

	unsigned int get_control_register(system_control::register_names register_name);
	void set_control_register(system_control::register_names register_name, unsigned int value);

	bool trigger_pending_interrupts(unsigned int & excode);

	void register_system_control_device(SystemControlInterface * interface)
	{
		system_control_devices[num_system_control_devices] = interface;
		num_system_control_devices++;
	}

private:
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

	unsigned int num_system_control_devices = 0;
	SystemControlInterface * system_control_devices[7] = { nullptr };
};