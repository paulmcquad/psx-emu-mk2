#pragma once
#include <memory>
#include <string>

class Bus;
class Cdrom;
class Gpu;
class Spu;
class Dma;
class IOPorts;
class Cpu;
class Ram;

class Psx
{
public:
	bool init(std::string bios_path);
	bool load(std::string bin_path, std::string cue_path);
	void tick();

	void save_state(std::string state_file_path);
	void load_state(std::string state_file_path);

	std::shared_ptr <Cdrom> bus = nullptr;
	std::shared_ptr <Cdrom> cdrom = nullptr;
	std::shared_ptr <Gpu> gpu = nullptr;
	std::shared_ptr <Spu> spu = nullptr;
	std::shared_ptr <Dma> dma = nullptr;
	std::shared_ptr <IOPorts> io_ports = nullptr;
	std::shared_ptr <Ram> ram = nullptr;
	std::shared_ptr <Cpu> cpu = nullptr;
};