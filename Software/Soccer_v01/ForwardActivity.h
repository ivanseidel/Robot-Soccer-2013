#include "SoccerLocator.h"
#include "MoveController.h"

#include <math.h>

#include <Wire.h>

#include <views/TextView.h>

#define compassAddress (0x42 >> 1)
#define TIME_LIMIT 100

#define X_BOUND 90
#define Y_BOUND 90

class ForwardActivity : public Activity
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
	TextView command;
	
	int currAngle;
	
	int posX, posY;
	
public:
	ForwardActivity(MoveController *_MC){
		Activity::Activity();
		ActivityName = String("Forward Player");
		
		MC = _MC;
		canMove = true;
		
		//Locator
		digitalWrite(pDIST_EN_TOP, HIGH);
		digitalWrite(pDIST_TRIG_TOP, HIGH);
		
		DistanceInterface *usSensors[4] = { new SharpLong(pDIST_1), 
											new SharpLong(pDIST_2), 
											new SharpLong(pDIST_3), 
											new SharpLong(pDIST_4) };
		locator = new SoccerLocator(0, usSensors);
		Threads.add(locator);
		
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
		
		ballView.o = Point(50, 350);
		intView.o = Point(150, 350);
		
		addView(&ballView);
		addView(&intView);
		
		runProgram.o = Point(200, 100);
		runProgram.w = 300;
		runProgram.h = 200;
		
		autoDestruct.o = Point(200, 350);
		
		addView(&runProgram);
		addView(&autoDestruct);
		
		command.o = Point(50, 400);
		addView(&command);
		
		delay(10);
		compassReading();
		delay(20);
		compassReading();
		delay(10);
		compassReading();
		delay(10);
		compassReading();
		delay(10);
		compassReading();
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
			delay(500);
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
	
	int getAngularSpeed() {
		int rawAngle = readRawAngle();
		
		//if (IMU->hasNewData) {
		if (!gotFirstAngle) {
			firstAngle = rawAngle;
			gotFirstAngle = true;
		}
		//}
		
		int angle = rawAngle;
		int diff = fixDegrees(angle - firstAngle);
		
		currAngle = diff;
		angles[0].setValue(currAngle);
		
		if (diff > 15)
			return max(min(-1*diff/2, -30), -80);
		else if (diff < -15)
			return min(max(-1*diff/2, 30), 80);
		else
			return 0;
	}
	
	int findMovementAngle(int ySpeed, int xSpeed) {
		int ang = atan2(ySpeed, xSpeed) * 180.0 / M_PI;
		return -ang - 90;
	}
	
	int getXSpeed() {
		posView.setValue(locator->posX());
		
		posX = locator->posX();
		
		if (locator->getDistance(LEFT) <= X_BOUND)
			return 255;
		if (locator->getDistance(RIGHT) <= X_BOUND)
			return -255;
		
		if (!ballVisible) {
			return 5 * (90 - posX);
		}
		
		if (abs(ballAngle) <= 30) {
			return 255 * sin((double) ballAngle * M_PI / 180.0);
		}
		
		if (ballAngle >= 90 || ballAngle <= -90) {
			if (ballAngle > 0)
				return -140;
			else
				return 140;
		}
		
		if (ballAngle < 0)
			return -140;
		else
			return 140;
	}
	
	int getYSpeed() {
		yView.setValue(locator->posY());
		
		posY = locator->posY();
		
		if (locator->getDistance(BACK) <= Y_BOUND)
			return 255;
		if (locator->getDistance(FRONT) <= Y_BOUND)
			return -255;
		
		if (!ballVisible) {
			return 5 * (80 - posY);
		}
		
		if (ballAngle <= -180 + 45 || ballAngle >= 180 - 45) {
			return -130;
		}
		
		if (abs(ballAngle) <= 30) {
			return 255 * cos((double) ballAngle * M_PI / 180.0);
		}
		
		//if (abs(ballAngle) <= 70)
		//	return 80;
		
		return -120;
	}
	
	void updateBallAngle() {
		while (BALL_USART.available()) {
			char data = BALL_USART.read();
			//odd -> intensity
			if (data & 1) {
				ballIntensity = data;
				intView.setValue(ballIntensity);
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
					ballAngle = fixDegrees(ballAngle + 11);
					
					if (ballAngle == -105) {
						ballAngle = 0;
						ballVisible = false;
					}
				}
				ballView.setValue(ballAngle);
			}
		}
	}
	
	int compassPID() {
		static long lastMillis = 0;
		static int lastError = 0;
		
		int error = currAngle;
		int ans = -3 * error / 2 + (lastError - error) * 3;
		
		if (millis() - lastMillis >= 50) {
			lastMillis = millis();	
			lastError = error;
		}
		
		return ans;
	}

	void onLoop(){
		static bool wasPaused = false;
		static long frontMillis = 0;
		static long backMillis = 0;
		
		int angSpeed = getAngularSpeed();
		
		updateBallAngle();
		
		int movAngle = 0, movSpeed = 0;
		
		int front = locator->getDistance(FRONT);
		int left = locator->getDistance(LEFT);
		int back = locator->getDistance(BACK);
		int right = locator->getDistance(RIGHT);
		
		int sides = 35;
		
		if (ballIntensity >= 5) {
			if (abs(ballAngle) <= 45) {
				if (ballIntensity > 170) {
					movAngle = 3 * ballAngle;
					movSpeed = 100;
					
					command.setValue(String("Chasing close"));
				}
				else {
					movAngle = ballAngle;
					movSpeed = 100;
					
					command.setValue(String("Chasing far"));
				}
			}
			else {
				if (ballAngle < 0) {
					movAngle = ballAngle - 70;
					movSpeed = 100;
				}
				else {
					movAngle = ballAngle + 70;
					movSpeed = 100;
				}
					
				command.setValue(String("Normal"));
			}
		}
		else {
			movAngle = movSpeed = 0;
			
			command.setValue(String("Idle"));
		}
		
		angSpeed = compassPID();
		
		if (!runProgram.isChecked()) {
			onPause();
			wasPaused = true;
			return;
		}
		else if (wasPaused) {
			wasPaused = false;
			onResume();
		}
		
		int xSpeed = movSpeed * sin(toRads(movAngle));
		int ySpeed = movSpeed * cos(toRads(movAngle));
		
		if (back < sides && ySpeed < 0)
			ySpeed = 0;
		if (front < sides && ySpeed > 0)
			ySpeed = 0;
		if (left < sides && xSpeed < 0)
			xSpeed = 0;
		if (right < sides && xSpeed > 0)
			xSpeed = 0;
		
		if (xSpeed == 0 && ySpeed == 0)
			MC->setSpeed(0, 0, angSpeed);
		else
			MC->setSpeed(findMovementAngle(ySpeed, xSpeed), 
						 min(255, max(-255, hypot(ySpeed, xSpeed))), 
						 angSpeed);
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
