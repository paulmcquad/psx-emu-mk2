class Ram {
public:
	unsigned char& operator[](unsigned int address);

private:
	// four SRAM chips of 512KB
	unsigned char memory[1024 * 512 * 4] = { 0 };
	unsigned char parallel_port[1024 * 64] = { 0 };
	unsigned char scratch_pad[1024] = { 0 };
	unsigned char hardware_registers[1024 * 8] = { 0 };
	unsigned char bios[1024 * 512] = { 0 };

	unsigned char unmapped_memory = 0;
};