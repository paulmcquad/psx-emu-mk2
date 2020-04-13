#include <stdexcept>
#include "Spu.hpp"
#include "SDL.h"

//https://gist.github.com/armornick/3447121
bool Spu::init()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		return false;
	}
	initialized = true;

	return true;
}

unsigned char Spu::get(unsigned int address)
{
	//throw std::logic_error("not implemented");
	return 0;
}

void Spu::set(unsigned int address, unsigned char value)
{
	//throw std::logic_error("not implemented");
}