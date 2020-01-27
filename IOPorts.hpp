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

	// peripheral io ports
	JOY_DATA = 0x1F801040,
	JOY_STAT = 0x1F801044,
	JOY_MODE = 0x1F801048,
	JOY_CTRL = 0x1F80104A,
	JOY_BAUD = 0x1F80104E,
	SIO_DATA = 0x1F801050,
	SIO_STAT = 0x1F801054,
	SIO_MODE = 0x1F801058,
	SIO_CTRL = 0x1F80105A,
	SIO_MISC = 0x1F80105C,
	SIO_BAUD = 0x1F80105E,

	// memory control 2
	RAM_SIZE = 0x1F801060,

	// interrupt control
	I_STAT = 0x1F801070,
	I_MASK = 0x1F801074,

	// dma registers
	/*
		DMA0
		0x1F801080 - 0x1F80108F

		DMA1
		0x1F801090 - 0x1F80109F

		etc
	*/
	DMA0 = 0x1F801080,
	DMA1 = 0x1F801090,
	DMA2 = 0x1F8010A0,
	DMA3 = 0x1F8010B0,
	DMA4 = 0x1F8010C0,
	DMA5 = 0x1F8010D0,
	DMA6 = 0x1F8010E0,
	DPCR = 0x1F8010F0,
	DICR = 0x1F8010F4,

	// timers
	TIMER0 = 0x1F801100,
	TIMER1 = 0x1F801110,
	TIMER2 = 0x1F801120,

	// cdrom registers
	CDROM_REGISTERS = 0x1F801800,

	// memory control 3
	CACHE_CONTROL = 0xFFFE0130
};

class IOPorts
{
public:
	unsigned char* operator[] (unsigned int addr);
	unsigned char* operator[] (io_registers io_reg);

private:
	unsigned char data[1024 * 8];
};