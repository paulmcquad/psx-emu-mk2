#include <string>
#include <unordered_map>
#include <deque>

constexpr unsigned int MAIN_MEMORY_SIZE = 1024 * 512 * 4;
constexpr unsigned int PARALLEL_PORT_SIZE = 1024 * 64;
constexpr unsigned int SCRATCH_PAD_SIZE = 1024;
constexpr unsigned int IO_PORTS_SIZE = 1024 * 8;
constexpr unsigned int BIOS_SIZE = 1024 * 512;
constexpr unsigned int CACHE_CONTROL_SIZE = 512;

class Ram {
public:
	void init(std::string bios_filepath);

	unsigned char load_byte(unsigned int address);
	unsigned short load_halfword(unsigned int address);
	unsigned int load_word(unsigned int address);

	void store_byte(unsigned int address, unsigned char value);
	void store_halfword(unsigned int address, unsigned short value);
	void store_word(unsigned int address, unsigned int value);

private:

	unsigned char* get_byte(unsigned int address);

	// four SRAM chips of 512KB
	unsigned char memory[MAIN_MEMORY_SIZE] = { 0 };
	unsigned char parallel_port[PARALLEL_PORT_SIZE] = { 0 };
	unsigned char scratch_pad[SCRATCH_PAD_SIZE] = { 0 };
	unsigned char io_ports[IO_PORTS_SIZE] = { 0 };
	unsigned char bios[BIOS_SIZE] = { 0 };
	unsigned char cache_control[CACHE_CONTROL_SIZE] = { 0 };
};