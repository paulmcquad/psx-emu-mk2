#include <stdexcept>
#include <fstream>
#include "SystemControlCoprocessor.hpp"
#include "InstructionTypes.hpp"
#include "InstructionEnums.hpp"
#include "Bus.hpp"
#include "Cpu.hpp"
#include "Cdrom.hpp"

static SystemControlCoprocessor * instance = nullptr;

SystemControlCoprocessor * SystemControlCoprocessor::get_instance()
{
	if (instance == nullptr)
	{
		instance = new SystemControlCoprocessor();
	}

	return instance;
}

bool SystemControlCoprocessor::is_address_for_device(unsigned int address)
{
	if (address >= I_STAT_START && address < I_MASK_END)
	{
		return true;
	}
	return false;
}

unsigned char SystemControlCoprocessor::get_byte(unsigned int address)
{
	if (address >= I_STAT_START && address < I_STAT_END)
	{
		return interrupt_status_register.bytes[address - I_STAT_START];
	}
	else if (address >= I_MASK_START && address < I_MASK_END)
	{
		return interrupt_mask_register.bytes[address - I_MASK_START];
	}

	throw std::logic_error("out of bounds");
}

void SystemControlCoprocessor::set_byte(unsigned int address, unsigned char value)
{
	if (address >= I_STAT_START && address < I_STAT_END)
	{
		interrupt_status_register.bytes[address - I_STAT_START] &= value;
	}
	else if (address >= I_MASK_START && address < I_MASK_END)
	{
		interrupt_mask_register.bytes[address - I_MASK_START] = value;
	}
	else
	{
		throw std::logic_error("out of bounds");
	}
}

unsigned int SystemControlCoprocessor::get_word(unsigned int address)
{
	if (address == I_STAT_START)
	{
		return interrupt_status_register.value;
	}
	else if (address == I_MASK_START)
	{
		return interrupt_mask_register.value;
	}
	else
	{
		throw std::logic_error("out of bounds");
	}
}

void SystemControlCoprocessor::set_word(unsigned int address, unsigned int value)
{
	if (address == I_STAT_START)
	{
		interrupt_status_register.value &= value;
	}
	else if (address == I_MASK_START)
	{
		interrupt_mask_register.value = value;
	}
	else
	{
		throw std::logic_error("out of bounds");
	}
}

SystemControlCoprocessor::SystemControlCoprocessor()
{
	interrupt_status_register.value = 0x0;
	interrupt_mask_register.value = 0x0;
	control_registers[static_cast<unsigned int>(system_control::register_names::PRID)] = 0x00000002;
}

SystemControlCoprocessor::~SystemControlCoprocessor()
{
}

void SystemControlCoprocessor::save_state(std::stringstream& file)
{
	file.write(reinterpret_cast<char*>(&control_registers[0]), sizeof(unsigned int) * 32);
	// todo save queue of interrupts
}

void SystemControlCoprocessor::load_state(std::stringstream& file)
{
	file.read(reinterpret_cast<char*>(&control_registers[0]), sizeof(unsigned int) * 32);

	// todo load queue of interrupts
}

void SystemControlCoprocessor::execute(const instruction_union& instruction)
{
	switch (static_cast<cpu_instructions>(instruction.immediate_instruction.op))
	{
		// LWC0
		case cpu_instructions::LWC0:
		{
			load_word_to_cop(instruction);
		} return;

		case cpu_instructions::SWC0:
		{
			store_word_from_cop(instruction);
		} return;
	}
	
	if (instruction.register_instruction.rs == 0b10000)
	{
		cop0_instructions func = static_cast<cop0_instructions>(instruction.register_instruction.funct);
		switch (func) {
			case cop0_instructions::RFE:
			{
				restore_from_exception(instruction);
			} break;
			default:
				throw std::logic_error("not supported on cop0");
		}
	}
	else
	{
		copz_instructions function = static_cast<copz_instructions>(instruction.register_instruction.rs);
		switch (function) {
			case copz_instructions::MF:
			{
				move_from_cop(instruction);
			} break;

			case copz_instructions::CF:
			{
				move_control_from_cop(instruction);
			} break;

			case copz_instructions::MT:
			{
				move_to_cop(instruction);
			} break;

			case copz_instructions::CT:
			{
				move_control_to_cop(instruction);
			} break;
		}
	}
}

unsigned int SystemControlCoprocessor::get_control_register(system_control::register_names register_name)
{
	unsigned int index = static_cast<unsigned int>(register_name);
	return control_registers[index];
}

void SystemControlCoprocessor::set_control_register(system_control::register_names register_name, unsigned int value)
{
	unsigned int index = static_cast<unsigned int>(register_name);
	control_registers[index] = value;
}

unsigned int SystemControlCoprocessor::get_control_register(unsigned int index)
{
	return control_registers[index];
}

void SystemControlCoprocessor::set_control_register(unsigned int index, unsigned int value)
{
	control_registers[index] = value;
}

// LWCz rt, offset(base)
void SystemControlCoprocessor::load_word_to_cop(const instruction_union& instr) 
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)Cpu::get_instance()->register_file.get_register(instr.immediate_instruction.rs);
	unsigned int word = Bus::get_instance()->get_word(addr);
	set_control_register(instr.immediate_instruction.rt, word);
}

// SWCz rt, offset(base)
void SystemControlCoprocessor::store_word_from_cop(const instruction_union& instr)
{
	unsigned int addr = (short)instr.immediate_instruction.immediate + (int)Cpu::get_instance()->register_file.get_register(instr.immediate_instruction.rs);
	unsigned int control_value = get_control_register(instr.immediate_instruction.rt);
	Bus::get_instance()->set_word(addr, control_value);
}

// MTCz rt, rd
void SystemControlCoprocessor::move_to_cop(const instruction_union& instr)
{
	unsigned int value = Cpu::get_instance()->register_file.get_register(instr.register_instruction.rt);
	set_control_register(instr.register_instruction.rd, value);
}

// MFCz rt, rd
void SystemControlCoprocessor::move_from_cop(const instruction_union& instr)
{
	unsigned int value = get_control_register(instr.register_instruction.rd);
	Cpu::get_instance()->register_file.set_register(instr.register_instruction.rt, value);
}

// CTCz rt, rd
void SystemControlCoprocessor::move_control_to_cop(const instruction_union& instr)
{
	move_to_cop(instr);
}

// CFCz rt, rd
void SystemControlCoprocessor::move_control_from_cop(const instruction_union& instr)
{
	move_from_cop(instr);
}

// COPz cofun
void SystemControlCoprocessor::move_control_to_cop_fun(const instruction_union& instr)
{
	throw std::logic_error("not supported on cop0");
}

void SystemControlCoprocessor::move_to_cp0(const instruction_union& instr)
{
	move_to_cop(instr);
}

void SystemControlCoprocessor::move_from_cp0(const instruction_union& instr)
{
	move_from_cop(instr);
}

void SystemControlCoprocessor::restore_from_exception(const instruction_union& instr)
{
	system_control::status_register sr = get_control_register(system_control::register_names::SR);

	// pop back the interrupt mode bits
	sr.IEc = sr.IEp;
	sr.KUc = sr.KUp;

	sr.IEp = sr.IEo;
	sr.KUp = sr.KUo;

	set_control_register(system_control::register_names::SR, sr.raw);
}

void SystemControlCoprocessor::set_irq_bits(unsigned int irq_bits)
{
	interrupt_status_register.IRQ_BITS |= irq_bits;
	system_control::cause_register cause = get_control_register(system_control::register_names::CAUSE);
	cause.Ip = 0x4; // apparently the ps1 only uses 1 interrupt line and from looking at the Im of the status register, it appears to be 0x4
	set_control_register(system_control::register_names::CAUSE, cause.raw);
}

void SystemControlCoprocessor::trigger_interrupts()
{
	system_control::status_register sr = get_control_register(system_control::register_names::SR);
	system_control::cause_register cause = get_control_register(system_control::register_names::CAUSE);

	if (cause.Ip == false) { 
		return; 
	}

	if (interrupt_status_register.IRQ_BITS & interrupt_mask_register.IRQ_BITS)
	{
		cause.Ip = 0;
		set_control_register(system_control::register_names::CAUSE, cause.raw);

		generate_interrupt(system_control::excode::INT);
	}
}

void SystemControlCoprocessor::generate_interrupt(system_control::excode excode)
{
	system_control::status_register sr = get_control_register(system_control::register_names::SR);
	system_control::cause_register cause = get_control_register(system_control::register_names::CAUSE);

	Cpu * cpu = Cpu::get_instance();

	if (cpu->in_delay_slot) {
		cause.BD = true;
		set_control_register(system_control::register_names::EPC, cpu->current_pc - 4);
	}
	else
	{
		set_control_register(system_control::register_names::EPC, cpu->current_pc);
	}

	if (sr.BEV == 0)
	{
		cpu->next_pc = static_cast<unsigned int>(system_control::exception_vector::GENERAL_BEV0);
	}
	else
	{
		cpu->next_pc = static_cast<unsigned int>(system_control::exception_vector::GENERAL_BEV1);
	}

	// push mode - these kind of act like a stack of exception states o = old, p = previous, c = current
	sr.IEo = sr.IEp;
	sr.KUo = sr.KUp;

	sr.IEp = sr.IEc;
	sr.KUp = sr.KUc;

	sr.IEc = sr.KUc = 0;

	set_control_register(system_control::register_names::SR, sr.raw);
	set_control_register(system_control::register_names::CAUSE, cause.raw);

	// dump the next instruction
	cpu->next_instruction = 0x0;
}