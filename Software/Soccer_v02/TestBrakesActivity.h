/*
Simple Activity to test motors and drivers
*/

#include "MoveController.h"
#include <views/TextView.h>
#include <views/AngleView.h>

class TestBrakesActivity : public Activity
{
protected:
	MoveController *MC;
	
	TextView speedViews[4];
	AngleView angleView;
	
	CompassPID *compassPID;

public:
	TestBrakesActivity(MoveController *_MC){
		Activity::Activity();
		MC = _MC;

		ActivityName = String("Test Brakes");
		
		for (int i = 0; i < 4; i++) {
			speedViews[i].o = Point(10 + i * 80, 10);
			addView(&speedViews[i]);
		}
		
		compassPID = new CompassPID();
		
		Threads.add(Compass);
		Threads.add(compassPID);
		
		angleView.o = Point(200, 200);
		angleView.w = 150;
		addView(&angleView);
	}

	virtual bool enablePause(){
		return false;
	}

	virtual void onResume(){
		// MC->setSpeed(0, 255, 0);
	}

	//Move motors using MoveDriver, if applicable
	virtual void onLoop(){
		static long currMillis = 0;
		static long lastMillis = 0;
		
		if (millis() - lastMillis < 50)
			return;
		
		if (currMillis == 0)
			currMillis = millis();
		
		int speed = 100;
		int theta = 0;
		
		long delta = millis() - currMillis;
		
		if (delta < 1000)
			theta = 0;
		else if (delta < 2000)
			theta = 90;
		else if (delta < 3000)
			theta = 180;
		else if (delta < 4000)
			theta = 270;
		else
			currMillis = millis();
		
		MC->setSpeed(theta, speed, compassPID->getThetaSpeed());
		
		angleView.setAngle(toRads(-Compass->readAngle()));
		
		//Serial.println("loop");
		
		/*MC->setMotorSpeeds(100, 0, 0, 0);
		delay(1000);
		MC->setMotorSpeeds(0, 100, 0, 0);
		delay(1000);
		MC->setMotorSpeeds(0, 0, 100, 0);
		delay(1000);
		MC->setMotorSpeeds(0, 0, 0, 100);
		delay(1000);*/
		
		/*MC->setSpeed(90, 100, 0);
		delay(1000);
		MC->setSpeed(0, 0, 0);
		delay(1000);
		MC->setSpeed(270, 220, 0);
		delay(1000);
		MC->setSpeed(0, 0, 0);
		delay(1000);
		MC->setSpeed(0, 100, 0);
		delay(1000);
		MC->setSpeed(0, 0, 0);
		delay(1000);
		MC->setSpeed(180, 100, 0);
		delay(1000);
		MC->setSpeed(0, 0, 0);
		delay(2000);*/
		
		/*for (int i = 0; i < 4; i++)
			speedViews[i].setValue((int) (MC->speeds[i]));

		delay(35);*/
	}

	virtual void onPauseLoop(){
		
	}

	virtual void onPause(){
		MC->setSpeed(0, 0, 0);
	}
};
