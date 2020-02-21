#include <unordered_map>
#include <memory>

// https://problemkaputt.de/psx-spx.htm#memorymap
constexpr unsigned int MEMORY_CONTROL_1_SIZE = 36;
constexpr unsigned int MEMORY_CONTROL_2_SIZE = 4;
constexpr unsigned int PERIPHERAL_IO_SIZE = 30;

constexpr unsigned int SPU_CONTROL_SIZE = 64;
constexpr unsigned int SPU_VOICE_SIZE = 576;

// I_STAT_SIZE and I_MASK_SIZE only use the first 2 bytes
// and the next 2 in both are considered garbage areas
constexpr unsigned int I_STAT_SIZE = 4;
constexpr unsigned int I_MASK_SIZE = 4;

constexpr unsigned int TIMER_SIZE = 45;

class Gpu;
class Dma;

class IOPorts
{
public:
	void init(std::shared_ptr<Gpu> _gpu, std::shared_ptr<Dma> _dma);
	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);

private:
	std::shared_ptr<Gpu> gpu = nullptr;
	std::shared_ptr<Dma> dma = nullptr;

	unsigned char memory_control_1[MEMORY_CONTROL_1_SIZE] = {0};
	unsigned char memory_control_2[MEMORY_CONTROL_2_SIZE] = {0};
	unsigned char peripheral_io[PERIPHERAL_IO_SIZE] = { 0 };

	unsigned char spu_control[SPU_CONTROL_SIZE] = { 0 };
	unsigned char spu_voice_registers[SPU_VOICE_SIZE] = { 0 };
	
	unsigned char post = 0;
	
	unsigned char i_stat[I_STAT_SIZE] = { 0 };
	unsigned char i_mask[I_MASK_SIZE] = { 0 };

	unsigned char timers[TIMER_SIZE] = { 0 };
};