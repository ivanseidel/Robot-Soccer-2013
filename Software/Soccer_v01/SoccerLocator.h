/*
Class created to locate robot on the soccer field over time, given 4 distance sensors
*/

#ifndef SoccerLocator_h
#define SoccerLocator_h

#include <math.h>

#include "sensors/DistanceInterface.h"
#include "ThreadController.h"

#include "Definitions.h"

#define SENSOR_FRONT 0
#define SENSOR_LEFT 1
#define SENSOR_BACK 2
#define SENSOR_RIGHT 3

class ReaderThread : public Thread {
protected:
	DistanceInterface *sensor;

public:
	long distance;
	
	ReaderThread(DistanceInterface *_sensor) {
		Thread::Thread();
		sensor = _sensor;
	}
	
	void run() {
		distance = sensor->readDistance();
		runned();
	}
};

class TrustedFinder : public Thread {
protected:
	ReaderThread *readers[4];
	long lastVal[4];

public:
	bool isTrusted[4];
	long posX, posY;

	TrustedFinder(ReaderThread *_readers[4]) {
		Thread::Thread();
		for (int i = 0; i < 4; i++) {
			readers[i] = _readers[i];
			isTrusted[i] = true;
			lastVal[i] = readers[i]->distance;
		}
	}
	
	void run() {
		if (abs(readers[SENSOR_LEFT]->distance + readers[SENSOR_RIGHT]->distance - EXPECTED_X_SUM) > MAX_X_DIFF) {
			long leftDiff = abs(readers[SENSOR_LEFT]->distance - lastVal[SENSOR_LEFT]);
			long rightDiff = abs(readers[SENSOR_RIGHT]->distance - lastVal[SENSOR_RIGHT]);
			
			if (leftDiff > rightDiff && leftDiff > MAX_X_DIFF)
				isTrusted[SENSOR_LEFT] = false;
			else if (rightDiff > MAX_X_DIFF)
				isTrusted[SENSOR_RIGHT] = false;
		}
		else
			isTrusted[SENSOR_LEFT] = isTrusted[SENSOR_RIGHT] = true;
		
		int expected_y_sum = EXPECTED_Y_SUM_OUTSIDE;
		if (60 <= posX && posX <= 120)
			expected_y_sum = EXPECTED_Y_SUM_WITHIN;
		
		if (abs(readers[SENSOR_FRONT]->distance + readers[SENSOR_BACK]->distance - expected_y_sum) > MAX_Y_DIFF) {
			long backDiff = abs(readers[SENSOR_BACK]->distance - lastVal[SENSOR_BACK]);
			long frontDiff = abs(readers[SENSOR_FRONT]->distance - lastVal[SENSOR_FRONT]);
			
			if (backDiff > frontDiff && backDiff > MAX_Y_DIFF)
				isTrusted[SENSOR_BACK] = false;
			else if (frontDiff > MAX_Y_DIFF)
				isTrusted[SENSOR_FRONT] = false;
		}
		else
			isTrusted[SENSOR_BACK] = isTrusted[SENSOR_FRONT] = true;
		
		for (int i = 0; i < 4; i++)
			lastVal[i] = readers[i]->distance;
		
		runned();
	}
};

class PositionCalculator : public Thread {
protected:
	ReaderThread *readers[4];
	TrustedFinder *trusted;
	bool usedBack;

public:
	long posX, posY;

	PositionCalculator(ReaderThread *_readers[4], TrustedFinder *_trusted) {
		Thread::Thread();
		for (int i = 0; i < 4; i++)
			readers[i] = _readers[i];
		trusted = _trusted;
	}
	
	void calcPosX() {
		if (trusted->isTrusted[SENSOR_LEFT])
			posX = readers[SENSOR_LEFT]->distance + 7;
		else if (trusted->isTrusted[SENSOR_RIGHT])
			posX = 182 - (readers[SENSOR_RIGHT]->distance + 7);
		else if (readers[SENSOR_LEFT]->distance > readers[SENSOR_RIGHT]->distance)
			posX = readers[SENSOR_LEFT]->distance + 7;
		else
			posX = 182 - (readers[SENSOR_RIGHT]->distance + 7);
	}
	
	void calcPosY() {
		if (trusted->isTrusted[SENSOR_BACK] || true) {
			posY = readers[SENSOR_BACK]->distance + 6;
			usedBack = true;
		}
		else if (trusted->isTrusted[SENSOR_FRONT]) {
			posY = 243 - (readers[SENSOR_FRONT]->distance + 6);
			usedBack = false;
		}
		else {// if (readers[SENSOR_BACK]->distance > readers[SENSOR_FRONT]->distance) {
			posY = readers[SENSOR_BACK]->distance + 6;
			usedBack = true;
		}
		/*else {
			posY = 243 - (readers[SENSOR_FRONT]->distance + 6);
			usedBack = false;
		}*/
	}
	
	void run() {
		calcPosX();
		calcPosY();
		
		//Consider goal
		if (65 <= posX && posX <= 115) {
			if (usedBack)
				posY += 20;
			else
				posY -= 20;
		}
		
		trusted->posX = posX;
		trusted->posY = posY;
		
		runned();
	}
};

class SoccerLocator : public ThreadController {
protected:
	ReaderThread *readers[4];
	TrustedFinder *trusted;
	PositionCalculator *posCalc;

public:
	SoccerLocator(long _interval, DistanceInterface *_sensors[4]) : ThreadController(_interval) {
		for (int i = 0; i < 4; i++) {
			readers[i] = new ReaderThread(_sensors[i]);
			readers[i]->setInterval(READER_THREAD_INTERVAL);
			add(readers[i]);
		}
		
		trusted = new TrustedFinder(readers);
		trusted->setInterval(TRUSTED_THREAD_INTERVAL);
		add(trusted);
		
		posCalc = new PositionCalculator(readers, trusted);
		posCalc->setInterval(POSITION_THREAD_INTERVAL);
		add(posCalc);
	}
	
	long posX() {
		return posCalc->posX;
	}
	
	long posY() {
		return posCalc->posY;
	}
	
	long getDistance(int i) {
		return readers[i]->distance;
	}
	
	bool isTrusted(int i) {
		return trusted->isTrusted[i];
	}
};

#endif
