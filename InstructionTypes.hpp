struct immediate_instruction
{
	immediate_instruction(unsigned int instruction) {
		immediate = instruction;

		instruction >>= 16;
		rt = instruction;

		instruction >>= 5;
		rs = instruction;

		instruction >>= 5;
		op = instruction;
	}

	unsigned int op : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int immediate : 16;
};

struct jump_instruction
{
	jump_instruction(unsigned int instruction) {
		target = instruction;
		op = instruction >> 26;
	}

	unsigned int op : 6;
	unsigned int target : 26;
};

struct register_instruction
{
	register_instruction(unsigned int instruction) {
		funct = instruction;

		instruction >>= 6;
		shamt = instruction;
		
		instruction >>= 5;
		rd = instruction;
		
		instruction >>= 5;
		rt = instruction;

		instruction >>= 5;
		rs = instruction;

		instruction >>= 5;
		op = instruction;
	}

	unsigned int op : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int rd : 5;
	unsigned int shamt : 5;
	unsigned int funct : 6;
};