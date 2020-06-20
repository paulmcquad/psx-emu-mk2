enum class cdrom_command : unsigned char
{
	Sync = 0x00,
	Getstat = 0x01,
	Setloc = 0x02,
	Play = 0x03,
	Forward = 0x04,
	Backward = 0x05,
	ReadN = 0x06,
	MotorOn = 0x07,
	Stop = 0x08,
	Pause = 0x09,
	Init = 0x0A,
	Mute = 0x0B,
	Demute = 0x0C,
	Setfilter = 0x0D,
	Setmode = 0x0E,
	Getparam = 0x0F,
	GetlocL = 0x10,
	GetlocP = 0x11,
	SetSession = 0x12,
	GetTN = 0x13,
	GetTD = 0x14,
	SeekL = 0x15,
	SeekP = 0x16,
	SetClock = 0x17,
	GetClock = 0x18,
	// there is a whole host of sub_functions that test can call but 0x20 is the
	// only one the normal bios seems to call, which returns the cd bios date
	Test = 0x19,
	GetID = 0x1A,
	ReadS = 0x1B,
	Reset = 0x1C,
	GetQ = 0x1D,
	ReadTOC = 0x1E,
	VideoCD = 0x1F,
};

enum class cdrom_response_interrupts : unsigned char
{
	// INT0
	NO_RESPONSE = 0,
	// INT1
	SECOND_RESPONSE_READ = 1,
	// INT2
	SECOND_RESPONSE = 2,
	// INT3
	FIRST_RESPONSE = 3,
	// INT4
	DATA_END = 4,
	// INT5
	ERROR_CODE = 5,

	// N/A
	INT6 = 6,
	INT7 = 7
};

enum cdrom_response_timings
{
	FIRST_RESPONSE_DELAY = 0xC4E1,
	SECOND_REPONSE_DELAY = 0x4a00
};