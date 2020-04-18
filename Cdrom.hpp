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

	bool load(std::string bin_path, std::string cue_path);
private:
	// https://problemkaputt.de/psx-spx.htm#cdromcontrollerioports
	union
	{
		unsigned char raw;
		struct
		{
			// Command/parameter transimission busy
			unsigned int BUSYSTS : 1;
			// Data fifo empty
			unsigned int DRQSTS : 1;
			// Response fifo empty
			unsigned int RSLRRDY : 1;
			// Parameter fifo full
			unsigned int PRMWRDY : 1;
			// Parameter fifo empty
			unsigned int PRMEMPT : 1;
			// XA-ADPCM fifo empty
			unsigned int ADPBUSY : 1;
			// port
			unsigned int INDEX : 2;
		} values;
		
	} status_register;

	// R/W when index = 1, R when index = 3
	union {
		unsigned int raw : 8;
		struct
		{
			// always 1
			unsigned int na4 : 1;
			unsigned int na3 : 1;
			unsigned int na2 : 1;

			unsigned int command_start : 1;

			// usually 0
			unsigned int na1 : 1;
			unsigned int response_received : 3;
		} values;
	} interrupt_flag_register;

	union {
		unsigned int raw : 8;
		struct
		{
			unsigned int na3 : 3;
			unsigned int interrupt_enable_bits : 5;
		} values;
	} interrupt_enable_register;

	unsigned int num_sectors = 0;
	std::vector<unsigned char> rom_data;
};