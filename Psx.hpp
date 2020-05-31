#pragma once
#include <memory>
#include <string>
#include <sstream>

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
class Timers;

class Psx
{
public:
	bool init(std::string bios_path);
	bool load(std::string bin_path, std::string cue_path);
	void tick();
	void reset();

	void save_state(std::stringstream& state_stream);
	void load_state(std::stringstream& state_stream);

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
	std::shared_ptr <Timers> timers = nullptr;
};