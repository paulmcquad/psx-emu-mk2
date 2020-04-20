#include <string>
#include <vector>

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

	std::vector<unsigned char> data_fifo;
	std::vector<unsigned char> response_fifo;
};