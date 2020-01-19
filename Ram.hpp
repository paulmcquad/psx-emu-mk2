#include <string>

class Ram {
public:
	void init(std::string bios_filepath);

	unsigned char* get_byte(unsigned int address);
	unsigned short* get_halfword(unsigned int address);
	unsigned int* get_word(unsigned int address);

private:
	// four SRAM chips of 512KB
	unsigned char memory[1024 * 512 * 4] = { 0 };

	unsigned char parallel_port[1024 * 64] = { 0 };
	unsigned char scratch_pad[1024] = { 0 };
	unsigned char hardware_registers[1024 * 8] = { 0 };
	unsigned char bios[1024 * 512] = { 0 };
};