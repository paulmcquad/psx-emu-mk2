#pragma once
#include <memory>
#include <string>

class Bus;
class Cdrom;
class Gpu;
class Spu;
class Dma;
class Cpu;
class Bus;
class Rom;
class Ram;
class MemoryControl;
class CacheControl;
class ParallelPort;

class Psx
{
public:
	bool init(std::string bios_path);
	bool load(std::string bin_path, std::string cue_path);
	void tick();

	void save_state(std::string state_file_path);
	void load_state(std::string state_file_path);

	std::shared_ptr <Cdrom> cdrom = nullptr;
	std::shared_ptr <Gpu> gpu = nullptr;
	std::shared_ptr <Spu> spu = nullptr;
	std::shared_ptr <Dma> dma = nullptr;
	std::shared_ptr <Bus> bus = nullptr;
	std::shared_ptr <Cpu> cpu = nullptr;
	std::shared_ptr <Ram> ram = nullptr;
	std::shared_ptr <Rom> rom = nullptr;
	std::shared_ptr <MemoryControl> memory_control = nullptr;
	std::shared_ptr <CacheControl> cache_control = nullptr;
	std::shared_ptr <ParallelPort> parallel_port = nullptr;
};