#include <string>
#include <vector>
#include <deque>

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

class Cdrom
{
public:
	void init();

	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);

	void tick();

	void trigger_pending_interrupts();

	unsigned char get_index0(unsigned int address);
	unsigned char get_index1(unsigned int address);
	unsigned char get_index2(unsigned int address);
	unsigned char get_index3(unsigned int address);

	void set_index0(unsigned int address, unsigned char value);
	void set_index1(unsigned int address, unsigned char value);
	void set_index2(unsigned int address, unsigned char value);
	void set_index3(unsigned int address, unsigned char value);

	bool load(std::string bin_path, std::string cue_path);
private:
	// https://problemkaputt.de/psx-spx.htm#cdromcontrollerioports
	union
	{
		unsigned char raw;
		struct
		{
			// port
			unsigned int INDEX : 2;
			// XA-ADPCM fifo empty
			unsigned int ADPBUSY : 1;
			// Parameter fifo empty
			unsigned int PRMEMPT : 1;
			// Parameter fifo full
			unsigned int PRMWRDY : 1;
			// Response fifo empty
			unsigned int RSLRRDY : 1;
			// Data fifo empty
			unsigned int DRQSTS : 1;
			// Command/parameter transimission busy
			unsigned int BUSYSTS : 1;
			
		};
		
	} status_register;

	unsigned int num_sectors = 0;
	std::vector<unsigned char> rom_data;

	// Audio controls
	// 1F801802h.Index2
	unsigned char volume_left_cd_to_left_spu = 0x0;
	// 1F801803h.Index2
	unsigned char volume_left_cd_to_right_spu = 0x0;
	// 1F801801h.Index3
	unsigned char volume_right_cd_to_right_spu = 0x0;
	// 1F801802h.Index3
	unsigned char volume_right_cd_to_left_spu = 0x0;
	// 1F801803h.Index3
	unsigned char volume_apply_changes = 0x0;
	// 1F801801h.Index1
	unsigned char sound_map_out = 0x0;
	// 1F801801h.Index2
	unsigned char sound_map_coding_info = 0x0;

	// 1F801802h.Index1 
	// 1F801803h.Index3
	union {
		unsigned char raw;
		struct
		{
			unsigned int response : 3;
			unsigned int unknown : 1;
			unsigned int command_start : 1;
			unsigned int na1 : 1;
			unsigned int na2 : 1;
			unsigned int na3 : 1;
		};
	} interrupt_flag_response_register;

	union {
		unsigned char raw;
		struct
		{
			unsigned int ack_1_7 : 3;
			unsigned int ack_8 : 1;
			unsigned int ack_10 : 1;
			unsigned int na1 : 1;
			unsigned int reset_param_fifo : 1;
			unsigned int na2 : 1;
		};
	} interrupt_flag_register;

	union {
		unsigned char raw;
		struct
		{
			unsigned int unknown : 3;
			unsigned int int_enable : 5;
		};
	} interrupt_enable_register;

	std::deque<unsigned char> response_fifo;
	std::deque<unsigned char> data_fifo;
	std::deque<unsigned char> parameter_fifo;

	std::deque<cdrom_response_interrupts> response_interrupt_queue;

	unsigned char get_next_response_byte();
	unsigned char get_next_data_byte();

	void execute_command(unsigned char command);
	void execute_test_command();
	void execute_getstat_command();
	void execute_getid_command();
};