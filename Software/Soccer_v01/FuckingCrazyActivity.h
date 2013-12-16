#include "SoccerLocator.h"
#include "MoveController.h"

#include <math.h>

#include <Wire.h>

#include <views/TextView.h>

#define compassAddress (0x42 >> 1)
#define TIME_LIMIT 100

#define X_BOUND 90
#define Y_BOUND 90

class FuckingCrazyActivity : public Activity
{
protected:
	SoccerLocator *locator;
	int firstAngle;
	bool gotFirstAngle;
	
	bool canMove;
	MoveController *MC;
	
	int ballAngle, ballIntensity;
	bool ballVisible;
	
	TextView angles[3];
	TextView posView, yView;
	
	CheckBox runProgram, autoDestruct;
	
	TextView ballView, intView;
	
	int currAngle;
	
	int posX, posY;
	
public:
	FuckingCrazyActivity(MoveController *_MC){
		Activity::Activity();
		ActivityName = String("Fucking Crazy Player");
		
		MC = _MC;
		canMove = true;
		
		//Locator
		digitalWrite(pDIST_EN_TOP, HIGH);
		digitalWrite(pDIST_TRIG_TOP, HIGH);
		
		DistanceInterface *usSensors[4] = { new EZUltrasonic(pDIST_6), 
											new EZUltrasonic(pDIST_7), 
											new EZUltrasonic(pDIST_8), 
											new EZUltrasonic(pDIST_9) };
		// locator = new SoccerLocator(0, usSensors);
		// Threads.add(locator);
		
		//IMU
		//Threads.add(IMU);
		Wire.begin();
		gotFirstAngle = false;
		
		//Ball
		ballAngle = 0;
		ballIntensity = 0;
		
		//Views
		for (int i = 0; i < 3; i++) {
			angles[i].o = Point(50 + 100 * i, 50);
			addView(&angles[i]);
		}
		
		posView.o = Point(50, 100);
		addView(&posView);
		
		yView.o = Point(150, 100);
		addView(&yView);
		
		ballView.o = Point(50, 150);
		intView.o = Point(150, 150);
		
		addView(&ballView);
		addView(&intView);
		
		runProgram.o = Point(200, 100);
		runProgram.w = 300;
		runProgram.h = 200;
		
		autoDestruct.o = Point(200, 350);
		
		addView(&runProgram);
		addView(&autoDestruct);
		
		delay(10);
		compassReading();
		delay(20);
		compassReading();
		delay(10);
	}
	
	void onCreate() {
		autoDestruct.setChecked(false);
		runProgram.setChecked(false);
	}

	void onResume(){
		//IMU->start();
		canMove = true;
		
		if (autoDestruct.isChecked()) {
			MC->setSpeed(180, 255, 0);
			delay(1500);
			MC->stop();
		}
	}
	
	int fixDegrees(int x) {
		while (x < -180)
			x += 360;
		while (x > 180)
			x -= 360;
		return x;
	}
	
	int compassReading() {
		int compassValue;
		long timeLimit_InitialTime;

		Wire.beginTransmission(compassAddress);
		Wire.write('A');
		Wire.endTransmission();

		Wire.requestFrom(compassAddress, 2);
  
		timeLimit_InitialTime = millis();

		while(Wire.available() < 2);/*
			  {
				 if((millis() - timeLimit_InitialTime) > TIME_LIMIT)
				 {
				   digitalWrite(LED_COMPASS_ERROR, HIGH);
				   motorStop();
				   motorClawRest();
				   delay(1000);
				 }
			  }*/
  
		compassValue = Wire.read();
		compassValue = compassValue << 8;
		compassValue += Wire.read();
  
		return compassValue/10;
	}
	
	int readRawAngle() {
		return compassReading();
	}

	void updateBallAngle() {
		while (BALL_USART.available()) {
			char data = BALL_USART.read();
			//odd -> intensity
			if (data & 1) {
				ballIntensity = data;
				if(ballIntensity == 1)
					ballVisible = false;
				//intView.setValue(ballIntensity);
			}
			else { //even -> shifted direction
				int direction = data >> 1;
				if (direction == 80) {
					ballAngle = 0;
					ballVisible = false;
				}
				else {
					ballVisible = true;
					direction = map(direction, 0, 31, 0, 360);
					
					direction -= 90;
					
					//Transform to match robot, 0 degree -> front
					ballAngle = -fixDegrees(-direction - 90);
					
					if (ballAngle == -105) {
						ballAngle = 0;
						ballVisible = false;
					}
				}
				//ballView.setValue(ballAngle);
			}
		}
	}

	int targetCompass;
	int compassPID(){
		static double compP = 0.0;
		static double compI = 0.0;

		compP = fixDegrees(targetCompass - compassReading()) * 1.5;
		compI += compP * 0.01;
		compI = max(min(compI, 150), -150);

		return /*compI + */compP;
	}


	void onLoop(){
		static bool wasPaused = false;
		long startTime = millis();
		
		if (!runProgram.isChecked()) {
			onPause();
			wasPaused = true;
			return;
		}
		else if (wasPaused) {
			wasPaused = false;
			onResume();
		}
		
		int compassCorrection = compassPID();

		MC->setSpeed(0,0,compassCorrection);

		// Wait until 50ms of
		// while(millis() - startTime < 50);
		Serial.println(millis() - startTime);
		delay(50);

	};

	void onStop(){
		//IMU->stop();
		//IMU->setDefaults();
		
		canMove = false;
		MC->stop();
	}
	
	void onPause() {
		canMove = false;
		MC->stop();
	}
};
