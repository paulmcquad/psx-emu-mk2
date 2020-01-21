enum class gte_commands : unsigned char
{
	RTPS = 0x01,
	NCLIP = 0x06,
	OP_sf = 0x0c,
	DPCS = 0x10,
	INTPL = 0x11,
	MVMVA = 0x12,
	NCDS = 0x13,
	CDP = 0x14,
	NCDT = 0x16,
	NCCS = 0x1b,
	NCS = 0x1e,
	NCT = 0x20,
	SQR_sf = 0x28,
	DCPL = 0x29,
	DPCT = 0x2a,
	AVSZ3 = 0x2d,
	AVSZ4 = 0x2e,
	RTPT = 0x30,
	GPF_sf = 0x3d,
	GPL_sf = 0x3e,
	NCCT = 0x3f
};

enum class copz_instructions : unsigned char
{
	MF = 000,
	CF = 002,
	MT = 004,
	CT = 006,
	BC = 010
};

enum class cop0_instructions : unsigned char
{
	TLBR = 001,
	TLBWI = 002,
	TLBWR = 006,
	TLBP = 010,
	RFE = 020
};

enum class cpu_instructions : unsigned char
{
	SPECIAL = 000,
	BCOND = 001,
	J = 002,
	JAL = 003,
	BEQ = 004,
	BNE = 005,
	BLEZ = 006,
	BGTZ = 007,
	ADDI = 010,
	ADDIU = 011,
	SLTI = 012,
	SLTIU = 013,
	ANDI = 014,
	ORI = 015,
	XORI = 016,
	LUI = 017,
	COP0 = 020,
	COP1 = 021,
	COP2 = 022,
	COP3 = 023,
	LB = 040,
	LH = 041,
	LWL = 042,
	LW = 043,
	LBU = 044,
	LHU = 045,
	LWR = 046,
	SB = 050,
	SH = 051,
	SWL = 052,
	SW = 053,
	SWR = 056,
	LWC0 = 060,
	LWC1 = 061,
	LWC2 = 062,
	LWC3 = 063,
	SWC0 = 070,
	SWC1 = 071,
	SWC2 = 072,
	SWC3 = 073
};

enum class cpu_special_funcs : unsigned char
{
	SLL = 000,
	SRL = 002,
	SRA = 003,
	SLLV = 004,
	SRLV = 006,
	SRAV = 007,
	JR = 010,
	JALR = 011,
	SYSCALL = 014,
	BREAK = 015,
	MFHI = 020,
	MTHI = 021,
	MFLO = 022,
	MTLO = 023,
	MULT = 030,
	MULTU = 031,
	DIV = 032,
	DIVU = 033,
	ADD = 040,
	ADDU = 041,
	SUB = 042,
	SUBU = 043,
	AND = 044,
	OR = 045,
	XOR = 046,
	NOR = 047,
	SLT = 052,
	SLTU = 053
};

enum class cpu_bconds : unsigned char
{
	BLTZ = 000,
	BGEZ = 001,
	BLTZAL = 020,
	BGEZAL = 021
};