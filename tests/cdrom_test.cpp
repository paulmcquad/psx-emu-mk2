#include <catch.hpp>
#include <memory>
#include "../Cdrom.hpp"
#include "../Exceptions.hpp"

TEST_CASE("Cdrom commands")
{
	unsigned int status_address = 0x1F801800 - CDROM_PORT_START;
	unsigned int command_address = 0x1F801801 - CDROM_PORT_START;
	unsigned int interrupt_enable_address = 0x1F801802 - CDROM_PORT_START;
	unsigned int interrupt_flag_address = 0x1F801803 - CDROM_PORT_START;
	unsigned int response_address = 0x1F801801 - CDROM_PORT_START;

	std::shared_ptr<Cdrom> cdrom = std::make_shared<Cdrom>();
	cdrom->init();
	cdrom->set(status_address, 1);
	cdrom->set(interrupt_enable_address, 0x1F);

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
		SECTION("Normal interrupt")
		{
			{
				unsigned int status_address = 0x1F801800 - CDROM_PORT_START;
				// index 0 for command register
				cdrom->set(status_address, 0);
				cdrom->set(command_address, static_cast<unsigned char>(Cdrom::cdrom_command::Getstat));
			}

			// index 1 for interrupt flag register
			{
				cdrom->set(status_address, 1);
				Cdrom::interrupt_flag_register_read response = cdrom->get(interrupt_flag_address);
				REQUIRE(response.response_received == static_cast<unsigned int>(Cdrom::cdrom_response_interrupts::FIRST_RESPONSE));
				REQUIRE(cdrom->response_fifo->get_current_size() == 1);
			}

			// trigger the interrupt
			{
				for (unsigned int idx = 0; idx < static_cast<unsigned int>(Cdrom::cdrom_response_timings::FIRST_RESPONSE_DELAY); idx++)
				{
					cdrom->tick();
				}

				REQUIRE_THROWS_AS(cdrom->trigger_pending_interrupts(), mips_interrupt);
				REQUIRE_NOTHROW(cdrom->trigger_pending_interrupts());
			}

			// check result
			{
				REQUIRE(cdrom->response_fifo->get_current_size() == 1);
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
		
		SECTION("Preempt interrupt")
		{
			{
				unsigned int status_address = 0x1F801800 - CDROM_PORT_START;
				// index 0 for command register
				cdrom->set(status_address, 0);
				cdrom->set(command_address, static_cast<unsigned char>(Cdrom::cdrom_command::Getstat));
			}

			// index 1 for interrupt flag register
			{
				cdrom->set(status_address, 1);
				Cdrom::interrupt_flag_register_read response = cdrom->get(interrupt_flag_address);
				REQUIRE(response.response_received == static_cast<unsigned int>(Cdrom::cdrom_response_interrupts::FIRST_RESPONSE));
				REQUIRE(cdrom->response_fifo->get_current_size() == 1);
			}

			// check result
			{
				REQUIRE(cdrom->response_fifo->get_current_size() == 1);
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

			// should be no interrupt if we acknowledge first
			{
				REQUIRE(cdrom->response_interrupt_queue.size() == 0);

				for (unsigned int idx = 0; idx < static_cast<unsigned int>(Cdrom::cdrom_response_timings::FIRST_RESPONSE_DELAY); idx++)
				{
					cdrom->tick();
				}

				REQUIRE_NOTHROW(cdrom->trigger_pending_interrupts());
			}
		}
	}

	SECTION("Getloc")
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

	SECTION("GetID")
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