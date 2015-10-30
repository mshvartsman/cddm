#include "catch_main.h"
#include "../fatal_error.h"

TEST_CASE("Fatal error throws correctly!"){

	REQUIRE_THROWS(throw fatal_error() << "TESTING ERROR!"); 

}