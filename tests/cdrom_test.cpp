#include <catch.hpp>
#include <memory>
#include "../Cdrom.hpp"
#include "../Cpu.hpp"
#include "../Bus.hpp"
#include "../SystemControlCoprocessor.hpp"

TEST_CASE("Cdrom commands")
{
	unsigned int status_address = 0x1F801800;
	unsigned int command_address = 0x1F801801;
	unsigned int interrupt_enable_address = 0x1F801802;
	unsigned int interrupt_flag_address = 0x1F801803;
	unsigned int response_address = 0x1F801801;

	std::shared_ptr<Cdrom> cdrom = std::make_shared<Cdrom>();
	cdrom->init();
	cdrom->set(status_address, 1);
	cdrom->set(interrupt_enable_address, 0x1F);

	std::shared_ptr <Cpu> cpu = std::make_shared<Cpu>();
	std::shared_ptr <Bus> bus = std::make_shared<Bus>();
	cpu->init(bus);

	cpu->cop0->interrupt_mask_register.IRQ2_CDROM = true;

	SECTION("Status register")
	{
		SECTION("Setting index")
		{
			for (int index = 3; index >= 0; index--)
			{
				cdrom->set(status_address, index);
				REQUIRE(cdrom->register_index == index);
				Cdrom::status_register_read status_result = cdrom->get(status_address);
				REQUIRE(status_result.INDEX == index);
			}
		}

		Cdrom::status_register_read status_result = cdrom->get(status_address);
		REQUIRE(status_result.ADPBUSY == 0);
		REQUIRE(status_result.PRMEMPT == 1);
		REQUIRE(status_result.PRMWRDY == 1);
		REQUIRE(status_result.RSLRRDY == 0);
		REQUIRE(status_result.DRQSTS == 0);
		REQUIRE(status_result.BUSYSTS == 0);
	}

	SECTION("Getstat")
	{
		{
			unsigned int status_address = 0x1F801800;
			// index 0 for command register
			cdrom->set(status_address, 0);
			cdrom->set(command_address, static_cast<unsigned char>(Cdrom::cdrom_command::Getstat));
		}

		// index 1 for interrupt flag register
		{
			cdrom->set(status_address, 1);
			Cdrom::interrupt_flag_register_read response = cdrom->get(interrupt_flag_address);
			REQUIRE(response.response_received == static_cast<unsigned int>(Cdrom::cdrom_response_interrupts::FIRST_RESPONSE));
		}

		// trigger the interrupt
		{
			for (unsigned int idx = 0; idx < static_cast<unsigned int>(Cdrom::cdrom_response_timings::FIRST_RESPONSE_DELAY); idx++)
			{
				cdrom->tick();
			}

			unsigned int excode = 0;
			REQUIRE(cdrom->trigger_pending_interrupts(cpu->cop0.get(), excode) == true);
		}

		// check result
		{
			cdrom->set(status_address, 1);
			unsigned char response = cdrom->get(response_address);
			REQUIRE(response == 0x2);
		}

		// acknowledge
		{
			Cdrom::interrupt_flag_register_write ack;
			ack.ack_int1_7 = static_cast<unsigned int>(Cdrom::cdrom_response_interrupts::FIRST_RESPONSE);
			cdrom->set(interrupt_flag_address, ack.raw);
		}
	}

	SECTION("GetID")
	{

	}

	SECTION("Setloc")
	{

	}

	SECTION("Play")
	{

	}

	SECTION("Forward")
	{

	}

	SECTION("Backward")
	{

	}

	SECTION("ReadN")
	{

	}

	SECTION("MotorOn")
	{

	}

	SECTION("Stop")
	{

	}

	SECTION("Pause")
	{

	}

	SECTION("Init")
	{

	}

	SECTION("Mute")
	{

	}

	SECTION("Demute")
	{

	}

	SECTION("Setfilter")
	{

	}

	SECTION("Setmode")
	{

	}

	SECTION("Getparam")
	{

	}

	SECTION("GetlocL")
	{

	}

	SECTION("GetlocP")
	{

	}

	SECTION("SetSession")
	{

	}

	SECTION("GetTN")
	{

	}

	SECTION("GetTD")
	{

	}

	SECTION("SeekL")
	{

	}

	SECTION("SeekP")
	{

	}

	SECTION("ReadS")
	{

	}

	SECTION("Reset")
	{

	}

	SECTION("GetQ")
	{

	}

	SECTION("ReadTOC")
	{

	}

	SECTION("Test - bios version")
	{

	}
}