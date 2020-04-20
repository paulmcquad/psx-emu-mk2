#include <string>
#include <vector>

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

class Cdrom
{
public:
	void init();

	void save_state(std::ofstream& file);
	void load_state(std::ifstream& file);

	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);

	void tick();

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
			
		} values;
		
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
		} values;
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
		} values;
	} interrupt_flag_register;

	union {
		unsigned char raw;
		struct
		{
			unsigned int unknown : 3;
			unsigned int int_enable : 5;
		} values;
	} interrupt_enable_register;

	struct
	{
		std::vector<unsigned char> data;
		int index = 0;

		void clear()
		{
			// todo
		}

		unsigned char get_next_byte()
		{
			// todo
			return 0;
		}
	} response_fifo;

	struct
	{
		std::vector<unsigned char> data;
		int index = 0;

		void clear()
		{
			// todo
		}

		unsigned char get_next_byte()
		{
			// todo
			return 0;
		}
	} data_fifo;

	struct
	{
		void set_next_byte(unsigned char value)
		{
			// todo
		}
	} parameter_fifo;

	unsigned char command_register = 0x0;
};