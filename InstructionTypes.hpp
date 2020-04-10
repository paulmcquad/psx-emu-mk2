#pragma once

// using technically undefined behaviour but it's generally supported
union instruction_union
{
	instruction_union(unsigned int value)
	{
		raw = value;
	}

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
		signed short x;
		signed short y;
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
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char op;
	};

	color_command(unsigned int val)
	{
		value = val;
	}
};

union draw_offset_command
{
	unsigned int value;
	struct
	{
		int x_offset : 11;
		int y_offset : 11;
		unsigned int na : 10;
	};

	draw_offset_command(unsigned int val)
	{
		value = val;
	}
};