#include <unordered_map>
#include <memory>

// https://problemkaputt.de/psx-spx.htm#memorymap
constexpr unsigned int MEMORY_CONTROL_1_SIZE = 36;
constexpr unsigned int MEMORY_CONTROL_2_SIZE = 4;
constexpr unsigned int PERIPHERAL_IO_SIZE = 30;

// I_STAT_SIZE and I_MASK_SIZE only use the first 2 bytes
// and the next 2 in both are considered garbage areas
constexpr unsigned int I_STAT_SIZE = 4;
constexpr unsigned int I_MASK_SIZE = 4;

constexpr unsigned int TIMER_SIZE = 45;

constexpr unsigned int CDROM_SIZE = 4;

class Spu;
class Gpu;
class Dma;
class Cdrom;

class IOPorts
{
public:
	void init(std::shared_ptr<Gpu> _gpu, std::shared_ptr<Dma> _dma, std::shared_ptr<Spu> _spu, std::shared_ptr<Cdrom> _cdrom);
	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	void tick();

	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);

	std::shared_ptr<Gpu> gpu = nullptr;
	std::shared_ptr<Dma> dma = nullptr;
	std::shared_ptr<Spu> spu = nullptr;
	std::shared_ptr<Cdrom> cdrom = nullptr;

	unsigned char memory_control_1[MEMORY_CONTROL_1_SIZE] = {0};
	unsigned char memory_control_2[MEMORY_CONTROL_2_SIZE] = {0};
	unsigned char peripheral_io[PERIPHERAL_IO_SIZE] = { 0 };
	
	unsigned char post = 0;

	union
	{
		unsigned bytes[4];
		struct
		{
			unsigned int IRQ0_VBLANK : 1;
			unsigned int IRQ1_GPU : 1;
			unsigned int IRQ2_CDROM : 1;
			unsigned int IRQ3_DMA : 1;
			unsigned int IRQ4_TMR0 : 1;
			unsigned int IRQ5_TMR1 : 1;
			unsigned int IRQ6_TMR2 : 1;
			unsigned int IRQ7_CTRL_MEM_CRD : 1;
			unsigned int IRQ8_SIO : 1;
			unsigned int IRQ9_SPU : 1;
			unsigned int IRQ10_LIGHTPEN : 1;
			unsigned int NA : 21;
		};
	} interrupt_status_register, interrupt_mask_register;

	unsigned char timers[TIMER_SIZE] = { 0 };

	union
	{
		unsigned char raw[2];
		struct
		{
			unsigned int tx_enable : 1;
			unsigned int joyn_output : 1;
			unsigned int rx_enable : 1;
			unsigned int unknown1 : 1;
			unsigned int ack : 1;
			unsigned int unknown2 : 1;
			unsigned int reset : 1;
			unsigned int na1 : 1;
			unsigned int rx_interrupt_mode : 2;
			unsigned int tx_interrupt_enable : 1;
			unsigned int rx_interrupt_enable : 1;
			unsigned int ack_interrupt_enable : 1;
			unsigned int desired_slot_number : 1;
			unsigned int na2 : 2;
		};
	} joy_ctrl;

	union
	{
		unsigned char raw[2];
	} joy_baud;

	union {
		unsigned char raw[2];
		struct
		{
			unsigned int baudrate_reload_factor : 2;
			unsigned int character_length : 2;
			unsigned int parity_enable : 1;
			unsigned int parity_type : 1;
			unsigned int unknown1 : 2;
			unsigned int clk_output_polarity : 1;
			unsigned int unknown2 : 7;
		};
	} joy_mode;
};