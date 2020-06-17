#include "Post.hpp"
#include <iostream>

static Post * instance = nullptr;

Post * Post::get_instance()
{
	if (instance == nullptr)
	{
		instance = new Post();
	}

	return instance;
}

bool Post::is_address_for_device(unsigned int address)
{
	if (address == 0x1f802041)
	{
		return true;
	}

	return false;
}

void Post::set_byte(unsigned int address, unsigned char value)
{
	std::cout << "Post: " << static_cast<int>(value) << std::endl;
}