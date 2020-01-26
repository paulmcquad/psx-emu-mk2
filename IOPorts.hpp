#include <unordered_map>

enum class io_registers : unsigned int
{
	// memory control 1
	EXPANSION_1_BASE_ADDRESS = 0x1F801000,
	EXPANSION_2_BASE_ADDRESS = 0x1F801004,
	EXPANSION_1_DELAY_SIZE = 0x1F801008,
	EXPANSION_3_DELAY_SIZE = 0x1F80100C,
	BIOS_ROM = 0x1F801010,
	SPU_DELAY = 0x1F801014,
	CDROM_DELAY = 0x1F801018,
	EXPANSION_2_DELAY_SIZE = 0x1F80101C,
	COM_DELAY = 0x1F801020,

	// memory control 3
	CACHE_CONTROL = 0xFFFE0130
};

class IOPorts
{
public:
	void init();
	unsigned char* operator[] (unsigned int addr);
	unsigned char* operator[] (io_registers io_reg);
	std::unordered_map<io_registers, unsigned char *> io_map;

private:
	unsigned char data[1024 * 8];
};