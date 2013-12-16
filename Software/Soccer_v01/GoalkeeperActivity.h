#include "SoccerLocator.h"
#include "MoveController.h"

#include <math.h>

#include <views/TextView.h>

//#include <Wire.h>

#define compassAddress (0x42 >> 1)
#define TIME_LIMIT 100
class GoalkeeperActivity : public Activity
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
	
	TextView ballView, intView;
	
	TextView firstView;
	
	int currAngle;
	
	bool outsideField;
	bool tooBack;
	
	int posX, posY;
	
public:
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
	
	GoalkeeperActivity(MoveController *_MC){
		Activity::Activity();
		ActivityName = String("Goalkeeper");
		
		MC = _MC;
		canMove = true;
		
		//Locator
		digitalWrite(pDIST_EN_TOP, HIGH);
		digitalWrite(pDIST_TRIG_TOP, HIGH);
		
		DistanceInterface *usSensors[4] = { new EZUltrasonic(pDIST_6), 
											new EZUltrasonic(pDIST_7), 
											new SharpLong(pDIST_3), 
											new EZUltrasonic(pDIST_9) };
		locator = new SoccerLocator(0, usSensors);
		Threads.add(locator);
		
		//IMU
		//Threads.add(IMU);
		gotFirstAngle = false;
		Wire.begin();
		
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
		
		firstView.o = Point(200, 200);
		addView(&firstView);
		
		delay(10);
		compassReading();
		delay(20);
		compassReading();
		delay(10);
	}

	void onResume(){
		//IMU->start();
		canMove = true;
	}
	
	int fixDegrees(int x) {
		while (x < -180)
			x += 360;
		while (x > 180)
			x -= 360;
		return x;
	}
	
	int readRawAngle() {
		compassReading();
		compassReading();
		return compassReading();
	}
	
	int getAngularSpeed() {	
		int rawAngle = readRawAngle();
		
		//if (IMU->hasNewData) {
		if (!gotFirstAngle) {
			firstAngle = rawAngle;
			gotFirstAngle = true;
			firstView.setValue(firstAngle);
		}
		//}
		
		int angle = rawAngle;
		int diff = fixDegrees(angle - firstAngle);
		
		currAngle = diff;
		angles[0].setValue(currAngle);
		
		if (diff > 10)
			return max(min(-2*diff/3, -30), -100);
		else if (diff < -10)
			return min(max(-2*diff/3, 30), 100);
		else
			return 0;
	}
	
	int getXSpeed() {
		posView.setValue(locator->posX());
		
		posX = locator->posX();
		
		if (70 <= locator->posX() && locator->posX() <= 110)
			outsideField = false;
		else
			outsideField = true;
		
		if (-35 <= currAngle && currAngle <= 35) {
			int targetX = 90;
			if (ballVisible) {
				if (ballAngle < -20) {
					if (posX > 70)
						targetX = posX - 45;
					else
						targetX = posX;
				}
				else if (ballAngle > 20) {
					if (posX < 110)
						targetX = posX + 45;
					else
						targetX = posX;
				}
				else
					targetX = posX;
			}
			
			return min(255, max(-255, 6*(targetX - locator->posX())));
		}
		else
			return 0;
	}
	
	int findMovementAngle(int ySpeed, int xSpeed) {
		int ang = atan2(ySpeed, xSpeed) * 180.0 / M_PI;
		return -ang - 90;
	}
	
	int getPosY() {
		return locator->getDistance(BACK);
	}
	
	int getYSpeed() {
		yView.setValue(getPosY());
		
		posY = getPosY();
		
		if (posX < 60 || posX > 120)
			return 50;
		
		if (posY <= 45)
			tooBack = true;
		else
			tooBack = false;
		
		if (-20 <= currAngle && currAngle <= 20)
			return min(255, max(-255, 3 * (35 - getPosY())));
		else
			return 0;
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
					
					if (ballAngle == -105) {
						ballAngle = 0;
						ballVisible = false;
					}
				}
				ballView.setValue(ballAngle);
			}
		}
	}
	
	bool ballApproaching() {
		return ballIntensity >= 100;
	}
	
	bool ballNear() {
		return ballIntensity >= 170;
	}

	void onLoop(){
		int angSpeed = getAngularSpeed();
		
		updateBallAngle();
		
		int front = locator->getDistance(FRONT);
		int left = locator->getDistance(LEFT);
		int back = locator->getDistance(BACK);
		int right = locator->getDistance(RIGHT);
		
	/*	
		//if (-30 <= ballAngle && ballAngle <= 30 && locator->getDistance(BACK) <= 60) {
		//	MC->setSpeed(ballAngle, 255, 0);
		//	return;
		//}
		
		int xSpeed = getXSpeed();
		int ySpeed = getYSpeed();
		
		/*
		if (outsideField && tooBack)
			xSpeed = 0;
		else if (outsideField && posY >= 55)
			ySpeed = 0;
		*/
		//xSpeed = ySpeed = angSpeed = 0;
		
		if (ballNear() && abs(ballAngle) <= 30) {
			MC->setSpeed(fixDegrees(ballAngle + 180), 255, 0);
			delay(1000);
			return;
		}
		
		int goalX = 0;
		if (ballApproaching()) {
			if (ballAngle < -15 && left > 45)
				goalX = -30;
			else if (ballAngle > 15 && right > 45)
				goalX = 30;
			else if (left > 45 && right > 45)
				goalX = right - left;
		}
		
		int xSpeed = min(180, max(-180, 6 * (right - left - goalX)));
		int ySpeed = min(255, max(-255, 3 * (30 - back)));
		
		if (abs(currAngle) > 15)
			xSpeed = ySpeed = 0;
		
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
		MC->setSpeed(0, 0, 0);
	}
};
