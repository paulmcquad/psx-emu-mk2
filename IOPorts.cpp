#include <typeinfo.h>
#include <fstream>
#include <iostream>
#include "IOPorts.hpp"
#include "Gpu.hpp"
#include "Spu.hpp"
#include "Cdrom.hpp"
#include "Dma.hpp"
#include "Exceptions.hpp"

constexpr unsigned int IO_START = 0x1F801000;
constexpr unsigned int GP0_Send_GPUREAD_START = 0x1f801810 - IO_START;
constexpr unsigned int GP0_Send_GPUREAD_END = GP0_Send_GPUREAD_START + 4;
constexpr unsigned int GP1_Send_GPUSTAT_START = 0x1f801814 - IO_START;
constexpr unsigned int GP1_Send_GPUSTAT_END = GP1_Send_GPUSTAT_START + 4;

constexpr unsigned int MEMORY_CONTROL_1_START = 0x1F801000 - IO_START;
constexpr unsigned int MEMORY_CONTROL_1_END = MEMORY_CONTROL_1_START + MEMORY_CONTROL_1_SIZE;

constexpr unsigned int MEMORY_CONTROL_2_START = 0x1F801060 - IO_START;
constexpr unsigned int MEMORY_CONTROL_2_END = MEMORY_CONTROL_2_START + MEMORY_CONTROL_2_SIZE;

constexpr unsigned int SPU_START = 0x1F801C00 - IO_START;
constexpr unsigned int SPU_END = SPU_START + SPU_VOICE_SIZE + SPU_CONTROL_SIZE;

constexpr unsigned int POST = 0x1F802041 - IO_START;

constexpr unsigned int I_STAT_START = 0x1F801070 - IO_START;
constexpr unsigned int I_STAT_END = I_STAT_START + I_STAT_SIZE;

constexpr unsigned int I_MASK_START = 0x1F801074 - IO_START;
constexpr unsigned int I_MASK_END = I_MASK_START + I_MASK_SIZE;

constexpr unsigned int TIMER_START = 0x1F801100 - IO_START;
constexpr unsigned int TIMER_END = TIMER_START + TIMER_SIZE;

constexpr unsigned int CDROM_START = 0x1F801800 - IO_START;
constexpr unsigned int CDROM_END = CDROM_START + CDROM_SIZE;

constexpr unsigned int DMA_SIZE = 128;
constexpr unsigned int DMA_START = 0x1F801080 - IO_START;
constexpr unsigned int DMA_END = DMA_START + DMA_SIZE;

constexpr unsigned int JOY_CTRL_START = 0x1F80104A - IO_START;
constexpr unsigned int JOY_CTRL_END = JOY_CTRL_START + 2;

void IOPorts::init(std::shared_ptr<Gpu> _gpu, std::shared_ptr<Dma> _dma, std::shared_ptr<Spu> _spu, std::shared_ptr<Cdrom> _cdrom)
{
	gpu = _gpu;
	dma = _dma;
	spu = _spu;
	cdrom = _cdrom;
}

void IOPorts::tick()
{

}

void IOPorts::save_state(std::ofstream& file)
{
	file.write(reinterpret_cast<char*>(&memory_control_1), sizeof(unsigned char) * MEMORY_CONTROL_1_SIZE);
	file.write(reinterpret_cast<char*>(&memory_control_2), sizeof(unsigned char) * MEMORY_CONTROL_2_SIZE);
	file.write(reinterpret_cast<char*>(&peripheral_io), sizeof(unsigned char) * PERIPHERAL_IO_SIZE);
	file.write(reinterpret_cast<char*>(&i_stat), sizeof(unsigned char) * I_STAT_SIZE);
	file.write(reinterpret_cast<char*>(&i_mask), sizeof(unsigned char) * I_MASK_SIZE);
	file.write(reinterpret_cast<char*>(&timers), sizeof(unsigned char) * TIMER_SIZE);
	file.write(reinterpret_cast<char*>(&post), sizeof(unsigned char));
	file.write(reinterpret_cast<char*>(&joy_ctrl), sizeof(unsigned char) * 2);
}

void IOPorts::load_state(std::ifstream& file)
{
	file.read(reinterpret_cast<char*>(&memory_control_1), sizeof(unsigned char) * MEMORY_CONTROL_1_SIZE);
	file.read(reinterpret_cast<char*>(&memory_control_2), sizeof(unsigned char) * MEMORY_CONTROL_2_SIZE);
	file.read(reinterpret_cast<char*>(&peripheral_io), sizeof(unsigned char) * PERIPHERAL_IO_SIZE);
	file.read(reinterpret_cast<char*>(&i_stat), sizeof(unsigned char) * I_STAT_SIZE);
	file.read(reinterpret_cast<char*>(&i_mask), sizeof(unsigned char) * I_MASK_SIZE);
	file.read(reinterpret_cast<char*>(&timers), sizeof(unsigned char) * TIMER_SIZE);
	file.read(reinterpret_cast<char*>(&post), sizeof(unsigned char));
	file.read(reinterpret_cast<char*>(&joy_ctrl), sizeof(unsigned char) * 2);
}

unsigned char IOPorts::get(unsigned int address)
{
	if (address >= GP0_Send_GPUREAD_START &&
		address < GP0_Send_GPUREAD_END)
	{
		return gpu->get(Gpu::GPUREAD, address - GP0_Send_GPUREAD_START);
	}
	else if (address >= GP1_Send_GPUSTAT_START &&
		address < GP1_Send_GPUSTAT_END)
	{
		return gpu->get(Gpu::GPUSTAT, address - GP1_Send_GPUSTAT_START);
	}
	else if (address >= MEMORY_CONTROL_1_START &&
		address < MEMORY_CONTROL_1_END)
	{
		return memory_control_1[address - MEMORY_CONTROL_1_START];
	}
	else if (address >= MEMORY_CONTROL_2_START &&
		address < MEMORY_CONTROL_2_END)
	{
		return memory_control_2[address - MEMORY_CONTROL_2_START];
	}
	else if (address >= SPU_START &&
		address < SPU_END)
	{
		return spu->get(address - SPU_START);
	}
	else if (address >= I_STAT_START &&
		address < I_STAT_END)
	{
		return i_stat[address - I_STAT_START];
	}
	else if (address >= I_MASK_START &&
		address < I_MASK_END)
	{
		return i_mask[address - I_MASK_START];
	}
	else if (address >= TIMER_START &&
		address < TIMER_END)
	{
		return timers[address - TIMER_START];
	}
	else if (address >= DMA_START &&
		address < DMA_END)
	{
		return dma->get(address - DMA_START);
	}
	else if (address == POST)
	{
		return post;
	}
	else if (address >= CDROM_START && address < CDROM_END)
	{
		return cdrom->get(address - CDROM_START);
	}
	else if (address >= JOY_CTRL_START && address < JOY_CTRL_END)
	{
		return joy_ctrl.raw[address - JOY_CTRL_START];
	}
	else
	{
		throw bus_error();
	}
}

void IOPorts::set(unsigned int address, unsigned char value)
{
	if (address >= GP0_Send_GPUREAD_START &&
		address < GP0_Send_GPUREAD_END)
	{
		gpu->set(Gpu::GP0_SEND, address - GP0_Send_GPUREAD_START, value);
	}
	else if (address >= GP1_Send_GPUSTAT_START &&
		address < GP1_Send_GPUSTAT_END)
	{
		gpu->set(Gpu::GP1_SEND, address - GP1_Send_GPUSTAT_START, value);
	}
	else if (address >= MEMORY_CONTROL_1_START &&
		address < MEMORY_CONTROL_1_END)
	{
		memory_control_1[address - MEMORY_CONTROL_1_START] = value;
	}
	else if (address >= MEMORY_CONTROL_2_START &&
		address < MEMORY_CONTROL_2_END)
	{
		memory_control_2[address - MEMORY_CONTROL_2_START] = value;
	}
	else if (address >= SPU_START &&
		address < SPU_END)
	{
		spu->set(address - SPU_START, value);
	}
	else if (address >= I_STAT_START &&
		address < I_STAT_END)
	{
		i_stat[address - I_STAT_START] = value;
	}
	else if (address >= I_MASK_START &&
		address < I_MASK_END)
	{
		i_mask[address - I_MASK_START] = value;
	}
	else if (address >= TIMER_START &&
		address < TIMER_END)
	{
		timers[address - TIMER_START] = value;
	}
	else if (address >= DMA_START &&
		address < DMA_END)
	{
		return dma->set(address - DMA_START, value);
	}
	else if (address == POST)
	{
		post = value;
	}
	else if (address >= CDROM_START && address < CDROM_END)
	{
		cdrom->set(address - CDROM_START, value);
	}
	else if (address >= JOY_CTRL_START && address < JOY_CTRL_END)
	{
		joy_ctrl.raw[address - JOY_CTRL_START] = value;
	}
	else
	{
		throw bus_error();
	}
}