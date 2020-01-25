#include <string>
#include <unordered_map>

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
	unsigned char memory[1024 * 512 * 4] = { 0 };
	unsigned char parallel_port[1024 * 64] = { 0 };
	unsigned char scratch_pad[1024] = { 0 };
	unsigned char hardware_registers[1024 * 8] = { 0 };
	unsigned char bios[1024 * 512] = { 0 };
	unsigned char cache_control[512] = { 0 };
};