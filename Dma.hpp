#pragma once
#include <memory>
#include <stdexcept>

#include "Bus.hpp"
#include "SystemControlCoprocessor.hpp"

class Gpu;
class Spu;

union DMA_base_address
{
	unsigned int int_value;
	unsigned char byte_value[4];
	struct
	{
		unsigned int memory_address : 24;
		unsigned int na : 8;
	};
};

union DMA_block_control
{
	unsigned int int_value;
	unsigned char byte_value[4];
	struct
	{
		unsigned int BC : 16;
		unsigned int na : 16;
	};
	struct
	{
		unsigned int BS : 16;
		unsigned int BA : 16;
	};
};

union DMA_channel_control
{
	unsigned int int_value;
	unsigned char byte_value[4];
	struct
	{
		unsigned int transfer_direction : 1;
		unsigned int memory_address_step : 1;
		unsigned int na1 : 6;
		unsigned int chopping_enable : 1;
		unsigned int sync_mode : 2;
		unsigned int na2 : 5;
		unsigned int chopping_dma_window_size : 3;
		unsigned int na3 : 1;
		unsigned int chopping_cpu_window_size : 3;
		unsigned int na4 : 1;
		unsigned int start_busy : 1;
		unsigned int na5 : 3;
		unsigned int start_trigger : 1;
		unsigned int unknown1 : 1;
		unsigned int unknown2 : 1;
		unsigned int na6 : 1;
	};
};

union DMA_control_register
{
	unsigned int int_value;
	unsigned char byte_value[4];
	struct
	{
		unsigned int MDECin_priority : 3;
		unsigned int MDECin_master_enable : 1;
		unsigned int MDECout_priority : 3;
		unsigned int MDECout_master_enable : 1;
		unsigned int GPU_priority : 3;
		unsigned int GPU_master_enable : 1;
		unsigned int CDROM_priority : 3;
		unsigned int CDROM_master_enable : 1;
		unsigned int SPU_priority : 3;
		unsigned int SPU_master_enable : 1;
		unsigned int PIO_priority : 3;
		unsigned int PIO_master_enable : 1;
		unsigned int OTC_priority : 3;
		unsigned int OTC_master_enable : 1;
		unsigned int unknown_priority : 3;
		unsigned int unknown_master_enable : 1;
	};
};

union DMA_interrupt_register
{
	unsigned int value;
	unsigned char byte_value[4];
	struct
	{
		unsigned int unknown : 6;
		unsigned int na : 9;
		unsigned int force_irq : 1;
		unsigned int irq_enable : 7;
		unsigned int irq_master_enable : 1;
		unsigned int irq_flags : 7;
		unsigned int irq_master_flag : 1;
	};
};

enum class DMA_sync_mode
{
	manual = 0,
	request = 1,
	linked_list = 2
};

enum class DMA_direction
{
	to_ram = 0,
	from_ram = 1
};

enum class DMA_address_step
{
	increment = 0,
	decrement = 1
};

enum class DMA_channel_type
{
	MDECin = 0,
	MDECout = 1,
	GPU = 2,
	CDROM = 3,
	SPU = 4,
	PIO = 5,
	OTC = 6
};

class DMA_interface
{
public:
	virtual void sync_mode_manual(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) { throw std::logic_error("not supported"); }
	virtual void sync_mode_request(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) { throw std::logic_error("not supported"); }
	virtual void sync_mode_linked_list(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) { throw std::logic_error("not supported"); }
};

class Dma : public DMA_interface, public Bus::BusDevice, public SystemControlCoprocessor::SystemControlInterface
{
public:
	virtual bus_device_type get_bus_device_type() final { return bus_device_type::DMA; }

	virtual bool is_address_for_device(unsigned int address) final;
	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;

	void init();
	void reset();
	void tick();
	void save_state(std::stringstream& file);
	void load_state(std::stringstream& file);

	// for OTC channel - since its basically DMA to ram
	virtual void sync_mode_manual(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;

	DMA_interface* devices[7] = { nullptr };
	DMA_interrupt_register interrupt_register;

	static Dma * get_instance();
private:
	static const unsigned int NUM_CHANNELS = 7;

	static const unsigned int DMA_SIZE = 128;
	static const unsigned int DMA_START = 0x1F801080;
	static const unsigned int DMA_END = DMA_START + DMA_SIZE;
	static const unsigned int DMA_BASE_ADDRESS_START = 0x1F801080;
	static const unsigned int DMA_BLOCK_CONTROL_START = 0x1F801084;
	static const unsigned int DMA_CHANNEL_CONTROL_START = 0x1F801088;
	static const unsigned int DMA_CONTROL_REGISTER_START = 0x1F8010F0;
	static const unsigned int DMA_INTERRUPT_REGISTER_START = 0x1F8010F4;
	static const unsigned int DMA_GARBAGE_START = 0x1F8010F8;

	unsigned char dma_registers[128] = { 0 };
	unsigned int * base_address_registers[7] = { nullptr };
	unsigned int * block_control_registers[7] = { nullptr };
	unsigned int * channel_control_registers[7] = { nullptr };
	unsigned int * control_register = nullptr;

	// Inherited via SystemControlInterface
	bool trigger_interrupt = false;
	virtual bool trigger_pending_interrupts(SystemControlCoprocessor * system_control_processor, unsigned int & excode) final;
};