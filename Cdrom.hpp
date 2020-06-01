#pragma once
#include <string>
#include <vector>
#include <deque>

#include "Fifo.hpp"
#include "Bus.hpp"
#include "SystemControlCoprocessor.hpp"

class Cdrom : public Bus::BusDevice, public SystemControlCoprocessor::SystemControlInterface
{
public:
	bool trigger_pending_interrupts(SystemControlCoprocessor* system_control_processor, unsigned int & excode) final;

	virtual bus_device_type get_bus_device_type() final { return bus_device_type::CDROM; }

	virtual bool is_peripheral() final { return true; }

	virtual bool is_address_for_device(unsigned int address) final;

	virtual unsigned char get_byte(unsigned int address) final;
	virtual void set_byte(unsigned int address, unsigned char value) final;

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

	enum class cdrom_response_timings : unsigned int
	{
		FIRST_RESPONSE_DELAY = 0xC4E1,
		SECOND_REPONSE_DELAY = 0x4a00
	};

	void init();
	~Cdrom();

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
	unsigned int current_response_received = 0;
	bool awaiting_acknowledgement = false;

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
	};

	// https://problemkaputt.de/psx-spx.htm#cdromcontrollerioports
	union status_register_read
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

		status_register_read()
		{
			raw = 0x0;
		}

		status_register_read(unsigned int value)
		{
			raw = value;
		}
	};

	unsigned int num_sectors = 0;
	std::vector<unsigned char> rom_data;

	Fifo<unsigned char> * response_fifo = nullptr;
	Fifo<unsigned char> * data_fifo = nullptr;
	Fifo<unsigned char> * parameter_fifo = nullptr;

	// pair delay, int type
	std::deque<std::pair<unsigned int, unsigned int>> response_interrupt_queue;

	unsigned char get_next_response_byte();
	unsigned char get_next_data_byte();

	void execute_command(unsigned char command);
	void execute_test_command();
	void execute_getstat_command();
	void execute_getid_command();
	void execute_read_toc_command();

	static const unsigned int CDROM_SIZE = 4;
	static const unsigned int CDROM_START = 0x1F801800;
	static const unsigned int CDROM_END = CDROM_START + CDROM_SIZE;

	// https://en.wikipedia.org/wiki/CD-ROM
	// http://rveach.romhack.org/PSXInfo/psx%20hardware%20info.txt
	// https://problemkaputt.de/psx-spx.htm#cdromdrive
	static const unsigned int SECTOR_SIZE = 2352;
	static const unsigned int STATUS_REGISTER = 0x1F801800;

	// command registers
	static const unsigned int COMMAND_REGISTER = 0x1F801801;
	static const unsigned int PARAMETER_FIFO_REGISTER = 0x1F801802;
	static const unsigned int REQUEST_REGISTER = 0x1F801803;

	static const unsigned int INTERRUPT_FLAG_REGISTER = 0x1F801803;
	static const unsigned int INTERRUPT_ENABLE_REGISTER_WRITE = 0x1F801802;
	static const unsigned int INTERRUPT_ENABLE_REGISTER_READ = 0x1F801803;

	static const unsigned int RESPONSE_FIFO_REGISTER = 0x1F801801;
	static const unsigned int DATA_FIFO_REGISTER = 0x1F801802;

	static const unsigned int RESPONSE_FIFO_SIZE = 16;
	static const unsigned int PARAMETER_FIFO_SIZE = 16;
	// double check
	static const unsigned int DATA_FIFO_SIZE = 4096;
	
};