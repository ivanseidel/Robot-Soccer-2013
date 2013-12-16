/*
Simple Activity to test motors and drivers
*/

#include "MoveController.h"

class TestMotorsActivity : public Activity
{
protected:
	bool canMove; //can put motors to movement?
	MoveController *MC;
	
public:
	TestMotorsActivity(MoveController *_MC){
		Activity::Activity();
		MC = _MC;

		ActivityName = String("Test Motors");
		canMove = false;
	}

	void onResume(){
		canMove = true;
	}

	//Move motors using MoveDriver, if applicable
	void onLoop(){
		if (canMove) {
			/*MD->setSpeed(1, 100);
			delay(1000);
			MD->setSpeed(1, -100);
			delay(1000);
			MD->setSpeed(1, 0);
			
			MD->setSpeed(2, 100);
			delay(1000);
			MD->setSpeed(2, -100);
			delay(1000);
			MD->setSpeed(2, 0);
			
			MD->setSpeed(3, 100);
			delay(1000);
			MD->setSpeed(3, -100);
			delay(1000);
			MD->setSpeed(3, 0);
			
			MD->setSpeed(4, 100);
			delay(1000);
			MD->setSpeed(4, -100);
			delay(1000);
			MD->setSpeed(4, 0);*/
			
			//MC->setSpeed(0, 0, 100); //angular speed only
			//MC->setSpeed(-120, 100, 0); //direction only
			
			/*MD->setSpeed(100, 0, 0, 0);
			delay(1000);
			MD->setSpeed(0, 100, 0, 0);
			delay(1000);
			MD->setSpeed(0, 0, 100, 0);
			delay(1000);
			MD->setSpeed(0, 0, 0, 100);
			delay(1000);
			MD->setSpeed(0, 0, 0, 0);
			delay(3000);*/

			MC->setSpeed(0, 255, 0); delay(1000);
			MC->setSpeed(90, 255, 0); delay(1000);
			MC->setSpeed(180, 255, 0); delay(1000);
			MC->setSpeed(-90, 255, 0); delay(1000);
			MC->setSpeed(0, 0, 0); delay(1000);
		}
		else
			MD->setSpeed(0, 0, 0, 0);
	};

	void onStop(){
		canMove = false;
		MD->setSpeed(0, 0, 0, 0);
	}
};
