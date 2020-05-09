#include <catch.hpp>
#include "Fifo.hpp"

TEST_CASE("Fifo test")
{
	Fifo<unsigned int> fifo(10);

	SECTION("Ensure values are correct")
	{
		REQUIRE(fifo.is_empty() == true);
		REQUIRE(fifo.is_full() == false);
		REQUIRE(fifo.get_max_size() == 10);
		REQUIRE(fifo.get_current_size() == 0);
	}

	SECTION("Pushing and popping")
	{
		fifo.push(1);
		fifo.push(2);
		fifo.push(3);
		fifo.push(4);
		fifo.push(5);
		fifo.push(6);
		fifo.push(7);
		fifo.push(8);
		fifo.push(9);
		fifo.push(10);

		REQUIRE(fifo.is_empty() == false);
		REQUIRE(fifo.is_full() == true);

		REQUIRE(fifo.peek() == 1);
		REQUIRE(fifo.peek(1) == 2);

		REQUIRE_THROWS_AS(fifo.push(11), std::out_of_range);

		REQUIRE(fifo.pop() == 1);
		REQUIRE(fifo.pop() == 2);
		REQUIRE(fifo.pop() == 3);
		REQUIRE(fifo.pop() == 4);
		REQUIRE(fifo.pop() == 5);
		REQUIRE(fifo.pop() == 6);
		REQUIRE(fifo.pop() == 7);
		REQUIRE(fifo.pop() == 8);
		REQUIRE(fifo.pop() == 9);

		REQUIRE(fifo.is_empty() == false);
		REQUIRE(fifo.is_full() == false);

		REQUIRE(fifo.pop() == 10);

		REQUIRE_THROWS_AS(fifo.pop(), std::out_of_range);

		// make sure it resizes
		fifo.push(11);
		REQUIRE(fifo.get_current_size() == 1);
		REQUIRE(fifo.pop() == 11);

		REQUIRE(fifo.is_empty() == true);
		REQUIRE(fifo.is_full() == false);
	}
}