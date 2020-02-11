#include <memory>

constexpr unsigned int DMA_SIZE = 128;

class Gpu;
class Ram;

class Dma
{
public:
	void init(std::shared_ptr<Ram> _ram, std::shared_ptr<Gpu> _gpu);
	unsigned char * operator[](unsigned int address);
private:

	std::shared_ptr<Ram> ram = nullptr;
	std::shared_ptr<Gpu> gpu = nullptr;

	unsigned char dma_registers[DMA_SIZE] = { 0 };
};