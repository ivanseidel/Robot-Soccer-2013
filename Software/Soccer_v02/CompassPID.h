#ifndef CompassPID_h
#define CompassPID_h

#include <Arduino.h>

#include <Thread.h>
#include <ArduinoSensors.h>
#include <interfaces/AngleInterface.h>

#define COMPASS_PID_INTERVAL 50

class CompassPID: public Thread {
protected:
	float targetAngle;
	float thetaSpeed;
	float lastError, integral;
	float errorK, derivativeK, integralK;

public:

	float targetDelta;
	
	CompassPID(float _errorK = 800, float _derivativeK = 1000, float _integralK = 200) {
		errorK = _errorK;
		derivativeK = _derivativeK;
		integralK = _integralK;
		
		lastError = 0;
		integral = 0;
		
		targetDelta = 0;
		
		Compass->readAngle();
		Compass->readAngle();
		Compass->readAngle();
		delay(10);
		Compass->readAngle();
		Compass->readAngle();
		Compass->readAngle();
		targetAngle = Compass->readAngle();
		
		Thread::Thread();
		setInterval(COMPASS_PID_INTERVAL);
	}
	
	float getError() {
		return fixDegrees(Compass->getAngle() - targetAngle);
	}
	
	void updatePID() {
		float error = fixDegrees(Compass->getAngle() - (targetAngle + targetDelta));
		
		float derivative = error - lastError;
		lastError = error;
		
		integral += error * ((float) COMPASS_PID_INTERVAL / 1000.0);
		integral = constrain(integral, -50, 50);
		
		float speed = error      * errorK      / 1000.0 + 
					  derivative * derivativeK / 1000.0 + 
					  integral   * integralK   / 1000.0;
		thetaSpeed = speed;
	}
	
	float getThetaSpeed() {
		return thetaSpeed;
	}

	/*
		For Thread Usage
	*/
	virtual void run(){
		updatePID();
		runned();
	}
};

#endif