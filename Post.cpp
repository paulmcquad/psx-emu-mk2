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

Post::bus_device_type Post::get_bus_device_type()
{
	return bus_device_type::POST;
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