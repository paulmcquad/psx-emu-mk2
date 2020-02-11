#include "Dma.hpp"

void Dma::init(std::shared_ptr<Ram> _ram, std::shared_ptr<Gpu> _gpu)
{
	ram = _ram;
	gpu = _gpu;
}

unsigned char * Dma::operator[](unsigned int address)
{
	return &dma_registers[address];
}