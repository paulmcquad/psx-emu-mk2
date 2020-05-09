#include <catch.hpp>
#include <memory>
#include "../Cdrom.hpp"

TEST_CASE("Cdrom commands")
{
	std::shared_ptr<Cdrom> cdrom = std::make_shared<Cdrom>();
	cdrom->init();

	SECTION("Status register")
	{
		unsigned int status_address = 0x1F801800 - CDROM_PORT_START;
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