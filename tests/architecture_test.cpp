#include "catch_main.h"
#include "../architecture.h"
#include "../config.h"


TEST_CASE("Random draws test for architecture"){
	Config conf = Config(); 
	// set time per setup, eblMean, eblSd, motorMean, motorSD
	conf.set<double>("timePerStep", 1);
	conf.set<double>("eblMean", 50); 
	conf.set<double>("motorPlanMean", 150);
	conf.set<double>("motorExecMean", 125);
	conf.set<double>("eblSd", 20); 
	conf.set<double>("motorSd", 50); 


	Architecture a = Architecture(&conf); 

	SECTION("Check that draws with timePerStep of 1 are (approximately) integers"){
		for (unsigned i=0; i<10; i++){
			double eblDraw = a.drawEBL(); 
			REQUIRE(floor(eblDraw) == Approx(eblDraw));
			double motorDraw = a.drawMotorPlanning(); 
			REQUIRE(floor(motorDraw) == Approx(motorDraw));
			motorDraw = a.drawMotorExec(); 
			REQUIRE(floor(motorDraw) == Approx(motorDraw));
		}
	}

}

