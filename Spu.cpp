#include <stdexcept>
#include "Spu.hpp"
#include "SDL.h"

static SDL_AudioDeviceID device_id;

//https://gigi.nullneuron.net/gigilabs/playing-a-wav-file-using-sdl2/
//https://gist.github.com/armornick/3447121
//https://github.com/leixiaohua1020/simplest_media_play/blob/master/simplest_audio_play_sdl2/simplest_audio_play_sdl2.cpp
bool Spu::init()
{
	spu_ram = new unsigned char[512 * 1042];

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		return false;
	}

	SDL_AudioSpec psx_audio_spec;
	psx_audio_spec.freq = 44100;
	psx_audio_spec.format = AUDIO_S16SYS;
	psx_audio_spec.channels = 2;
	psx_audio_spec.silence = 0;
	psx_audio_spec.samples = 1024;
	psx_audio_spec.callback = nullptr;

	SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(nullptr, 0, &psx_audio_spec, nullptr, 0);
	SDL_PauseAudioDevice(device_id, 0);

	initialized = true;

	return true;
}

Spu::~Spu()
{
	if (spu_ram)
	{
		delete[] spu_ram;
	}

	if (initialized)
	{
		SDL_CloseAudioDevice(device_id);
		SDL_Quit();
	}
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