#pragma once
#include <memory>
#include <stdexcept>

class Gpu;
class Ram;

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
	virtual void sync_mode_manual(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) { throw std::logic_error("not supported"); }
	virtual void sync_mode_request(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) { throw std::logic_error("not supported"); }
	virtual void sync_mode_linked_list(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) { throw std::logic_error("not supported"); }
};

class Dma : public DMA_interface
{
public:
	void init(std::shared_ptr<Ram> _ram, std::shared_ptr<Gpu> _gpu);
	void reset();
	void tick();
	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);

	// for OTC channel - since its basically DMA to ram
	virtual void sync_mode_manual(std::shared_ptr<Ram> ram, DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) override;

	DMA_interface* devices[7] = { nullptr };
private:

	std::shared_ptr<Ram> ram = nullptr;
	std::shared_ptr<Gpu> gpu = nullptr;

	unsigned char dma_registers[128] = { 0 };
	unsigned int * base_address_registers[7] = { nullptr };
	unsigned int * block_control_registers[7] = { nullptr };
	unsigned int * channel_control_registers[7] = { nullptr };
	unsigned int * control_register = nullptr;
	unsigned int * interrupt_register = nullptr;
};