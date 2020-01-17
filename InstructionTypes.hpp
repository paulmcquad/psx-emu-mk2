struct ImmediateInstruction
{
	unsigned int op : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int immediate : 16;
};

struct JumpInstruction
{
	unsigned int op : 6;
	unsigned int target : 26;
};

struct RegisterInstruction
{
	unsigned int op : 6;
	unsigned int rs : 5;
	unsigned int rt : 5;
	unsigned int rd : 5;
	unsigned int shamt : 5;
	unsigned int funct : 6;
};