#pragma once
#include <glm/glm.hpp>
#include <glm/common.hpp>

// using technically undefined behaviour but it's generally supported
union instruction_union
{
	unsigned int raw;
	struct
	{
		unsigned int immediate : 16;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int op : 6;
	} immediate_instruction;
	struct
	{
		unsigned int target : 26;
		unsigned int op : 6;
	} jump_instruction;
	struct
	{
		unsigned int funct : 6;
		unsigned int shamt : 5;
		unsigned int rd : 5;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int op : 6;
	} register_instruction;
};

// gpu command unions
union vert_command
{
	unsigned int value;
	struct
	{
		int x : 11;
		unsigned int na0 : 5;
		int y : 11;
		unsigned int na1 : 5;
	};

	vert_command(unsigned int val)
	{
		value = val;
	}
};

union dest_coord_command
{
	unsigned int value;
	struct
	{
		unsigned int x_pos : 10;
		unsigned int na0 : 6;
		unsigned int y_pos : 9;
		unsigned int na1 : 7;
	};

	dest_coord_command(unsigned int val)
	{
		value = val;
	}
};

union width_height_command
{
	unsigned int value;
	struct
	{
		unsigned int x_siz : 10;
		unsigned int na0 : 6;
		unsigned int y_siz : 9;
		unsigned int na1 : 7;
	};

	width_height_command(unsigned int val)
	{
		value = val;
	}
};

union color_command
{
	unsigned int value;
	struct
	{
		glm::u8vec3 bgr;
		unsigned char op;
	};

	color_command(unsigned int val)
	{
		value = val;
	}
};