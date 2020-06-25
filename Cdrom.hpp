#pragma once
#include <string>
#include <vector>
#include <deque>

#include "Fifo.hpp"
#include "Bus.hpp"
#include "SystemControlCoprocessor.hpp"

#include "Dma.hpp"

#include "CdromEnums.hpp"

class Cdrom : public Bus::BusDevice, public DMA_interface
{
public:
	static Cdrom * get_instance();

	// bus device functions
	virtual bool is_address_for_device(unsigned int address) final;

	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;

	// DMA interface functions
	virtual void sync_mode_manual(DMA_base_address& base_address, DMA_block_control& block_control, DMA_channel_control& channel_control) final;

	void init();

	void save_state(std::stringstream& file);
	void load_state(std::stringstream& file);

	unsigned char get(unsigned int address);
	void set(unsigned int address, unsigned char value);

	void tick();
	void reset();

	bool load(std::string bin_path, std::string cue_path);

	unsigned char get_index0(unsigned int address);
	unsigned char get_index1(unsigned int address);
	unsigned char get_index2(unsigned int address);
	unsigned char get_index3(unsigned int address);

	void set_index0(unsigned int address, unsigned char value);
	void set_index1(unsigned int address, unsigned char value);

	unsigned int register_index = 0;

	unsigned char request_register = 0x0;
	unsigned char interrupt_enable_register = 0x0;

	union interrupt_flag_register_write
	{
		unsigned char raw;
		struct
		{
			unsigned int ack_int1_7 : 3;
			unsigned int ack_int8 : 1;
			unsigned int ack_int10 : 1;
			unsigned int na1 : 1;
			unsigned int reset_param_fifo : 1;
			unsigned int na2 : 1;
		};

		interrupt_flag_register_write()
		{
			raw = 0x0;
		}

		interrupt_flag_register_write(unsigned char value)
		{
			raw = value;
		}
	};

	union interrupt_flag_register_read
	{
		unsigned char raw;
		struct
		{
			unsigned int response_received : 3;
			unsigned int na1 : 1;
			unsigned int command_start : 1;
			unsigned int na2 : 3;
		};

		interrupt_flag_register_read(unsigned char value)
		{
			raw = value;
		}

		interrupt_flag_register_read()
		{
			raw = 0x0;
			na2 = 0b111;
		}
	};

	// https://problemkaputt.de/psx-spx.htm#cdromcontrollerioports
	union status_register
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
			// Command/parameter transmission busy
			unsigned int BUSYSTS : 1;
		};

		status_register()
		{
			raw = 0x0;
		}

		status_register(unsigned int value)
		{
			raw = value;
		}
	};

	unsigned int num_sectors = 0;
	std::vector<unsigned char> rom_data;

	struct pending_response_data
	{
		int delay = 0;
		cdrom_response_interrupts int_type;
		std::vector<unsigned char> responses;
	};

	std::deque<pending_response_data> pending_response;

	cdrom_response_interrupts current_int = cdrom_response_interrupts::NO_RESPONSE;

	bool interrupt_countdown_active = false;
	int time_to_irq = 0;
	// keep throwing int1 until pause sent
	bool in_read_mode = false;

	Fifo<unsigned char> * response_fifo = nullptr;
	Fifo<unsigned char> * data_fifo = nullptr;
	Fifo<unsigned char> * parameter_fifo = nullptr;

	unsigned char get_next_response_byte();
	unsigned char get_next_data_byte();

	void read_data();

	void execute_command(unsigned char command);
	void execute_test_command();
	void execute_getstat_command();
	void execute_getid_command();
	void execute_read_toc_command();
	void execute_set_loc_command();
	void execute_seek_l_command();
	void execute_set_mode_command();
	void execute_read_n_command();
	void execute_pause_command();

	static const unsigned int CDROM_SIZE = 4;
	static const unsigned int CDROM_START = 0x1F801800;
	static const unsigned int CDROM_END = CDROM_START + CDROM_SIZE;

	// https://en.wikipedia.org/wiki/CD-ROM
	// http://rveach.romhack.org/PSXInfo/psx%20hardware%20info.txt
	// https://problemkaputt.de/psx-spx.htm#cdromdrive
    // https://byuu.net/compact-discs/structure
	static const unsigned int SECTOR_SIZE = 2352;
	static const unsigned int MODE1_USER_DATA_SIZE = 2048;

	static const unsigned int RESPONSE_FIFO_SIZE = 16;
	static const unsigned int PARAMETER_FIFO_SIZE = 16;
	// double check
	static const unsigned int DATA_FIFO_SIZE = 4096;

	struct location_data
	{
		unsigned char track = 0xAA;
		unsigned char index = 0x01;
		unsigned char mm = 0x00;
		unsigned char ss = 0x00;
		unsigned char sect = 0x00;
		unsigned char amm = 0x00;
		unsigned char ass = 0x00;
		unsigned char asect = 0x00;
	} seek_target, location;

	union
	{
		unsigned char raw = 0x0;
		struct
		{
			unsigned int cdda : 1;
			unsigned int auto_pause : 1;
			unsigned int report : 1;
			unsigned int xa_filter : 1;
			unsigned int ignore_bit : 1;
			unsigned int sector_size : 1;
			unsigned int xa_adpcm : 1;
			unsigned int speed : 1;
		};
	} mode;

private:

	Cdrom() = default;
	~Cdrom();
	
};