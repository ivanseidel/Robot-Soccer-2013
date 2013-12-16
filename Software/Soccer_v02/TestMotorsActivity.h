/*
Simple Activity to test motors and drivers
*/

#include "MoveController.h"
#include "views/CheckBox.h"

void TestMotorsActivity_enableBT(bool enable){

}

class TestMotorsActivity : public Activity
{
protected:
	MoveController *MC;

	CheckBox *enableBT;
	CheckBox *enableCompass;
	CheckBox *enableStopOnLine;
	AngleView *errorView;

	SeekBar *errorK;
	SeekBar *integralK;
	SeekBar *derivativeK;

	ProgressBar *lineStatus;

	float targetAngle;

	float error;
	float lastError;
	float integral;
	float derivative;

public:
	TestMotorsActivity(MoveController *_MC){
		Activity::Activity();
		MC = _MC;

		targetAngle = 0;
		integral = 0;

		enableBT = new CheckBox("BT Enable");
		enableBT->o = Point(20,20);
		enableBT->w = 200;
		enableBT->h = 60;
		addView(enableBT);

		enableCompass = new CheckBox("Regulate Heading");
		enableCompass->o = Point(20,100);
		enableCompass->w = 200;
		enableCompass->h = 60;
		addView(enableCompass);

		enableStopOnLine = new CheckBox("Stop on Line");
		enableStopOnLine->o = Point(20,100+80);
		enableStopOnLine->w = 200;
		enableStopOnLine->h = 60;
		addView(enableStopOnLine);

		integralK = new SeekBar();
		integralK->o = Point(20,180+80);
		integralK->w = 400;
		integralK->h = 40;
		integralK->setMax(1000);
		integralK->setMin(0);
		integralK->setValue(200);
		addView(integralK);

		errorK = new SeekBar();
		errorK->o = Point(20,180 + 60*2);
		errorK->w = 400;
		errorK->h = 40;
		errorK->setMax(2000);
		errorK->setMin(0);
		errorK->setValue(800);
		addView(errorK);

		lineStatus = new ProgressBar();
		lineStatus->o = Point(600,180 + 60*2);
		lineStatus->w = 150;
		lineStatus->h = 40;
		lineStatus->setMax(1);
		addView(lineStatus);

		derivativeK = new SeekBar();
		derivativeK->o = Point(20,180 + 60*3);
		derivativeK->w = 400;
		derivativeK->h = 40;
		derivativeK->setMax(2000);
		derivativeK->setMin(-2000);
		derivativeK->setValue(1000);
		addView(derivativeK);

		errorView = new AngleView();
		errorView->o = Point(650,120);
		errorView->w = 100;
		addView(errorView);

		ActivityName = String("Test Motors");

		Threads.add(Compass);
	}

	virtual bool enablePause(){
		return false;
	}

	virtual void onResume(){
		// MC->setSpeed(0, 255, 0);
	}


	long lastRecCommand;
	long lastLoop;
	bool shouldBeep;

	float finalTheta;
	float finalSpeed;
	float finalThetaSpeed;

	//Move motors using MoveDriver, if applicable
	virtual void onLoop(){

		// Calculate deltaTime
		int deltaTime = millis() - lastLoop;
		lastLoop = millis();

		// Print loop time
		Serial.print("UPDT: ");
		Serial.println(deltaTime);
		
		Serial.println(Compass->getAngle());

		if(deltaTime > 100){
			deltaTime = 0;
		}

		if(enableBT->isChecked()){
			if(BT.available() >= 3){

				int8_t buff[3];
				buff[0] = BT.read();

				if((buff[0] & 0b10000000) > 0){

					buff[1] = BT.read();
					buff[2] = BT.read();

					int xVector =
						(int)(buff[0] & 0b00111111) *
						(int)(((buff[0] & 0b01000000) == 0) * 2 - 1);

					int yVector =
						(int)(buff[1] & 0b00111111) *
						(int)(((buff[1] & 0b01000000) == 0) * 2 - 1);

					int rotation =
						(int)(buff[2] & 0b00111111) *
						(int)(((buff[2] & 0b01000000) == 0) * 2 - 1);

					float angle = toDegs(atan2(xVector, yVector));
					float force = sqrt(xVector*xVector + yVector*yVector)*8;
					float thetaSpeed = rotation;

					finalTheta = angle;
					finalSpeed = force;
					finalThetaSpeed = thetaSpeed*6;

					// Set the target speed, case compass is checked
					if(enableCompass->isChecked()){
						if(abs(rotation) > 5){
							targetAngle += rotation*(deltaTime/1000.0)*40;
							LineSensor->reset();
						}
					}

					// Connected!
					if(millis() - lastRecCommand > 2000){
						beep(3);
						integral = 0;
					}
					lastRecCommand = millis();
					shouldBeep = true;

				}
				
				// Clear Buffer
				while(BT.available())
					BT.read();
				
			}

			/*
				Check if bluetooth is inactive for more than 2 seconds
			*/
			if(millis() - lastRecCommand > 2000 && shouldBeep){
				finalTheta = 0;
				finalSpeed = 0;
				finalThetaSpeed = 0;

				beep(2);
				shouldBeep = false;
			}

		}else{
			finalTheta = 0;
			finalSpeed = 0;
			finalThetaSpeed = 0;
		}

		if(enableCompass->isChecked()){
			error = fixDegrees(Compass->getAngle() - targetAngle) * 1.0;
			errorView->setAngle(toRads(error));

			derivative = (error - lastError);
			lastError = error;

			integral += error * (deltaTime/1000.0);
			// PC.print("TARGET: ");
			// PC.print(targetAngle);
			// PC.print("\tERROR: ");
			// PC.println(error);

			integral = constrain(integral, -50, 50);

			finalThetaSpeed = 
				error * (errorK->getValue()/1000.0) +
				integral * (integralK->getValue()/1000.0) +
				derivative * (derivativeK->getValue()/1000.0);

			if(finalThetaSpeed > 50)
				finalThetaSpeed = 50;
			else if(finalThetaSpeed < -50)
				finalThetaSpeed = -50;
		}

		lineStatus->setValue(LineSensor->readValue());

		if(enableBT->isChecked() && enableStopOnLine->isChecked() && LineSensor->readValue()){
			MC->setSpeed(0,0,0);
		}else{
			MC->setSpeed(finalTheta, finalSpeed, finalThetaSpeed);
		}

		

		delay(35);
	}

	virtual void onPauseLoop(){
		
	}

	virtual void onPause(){
		MC->setSpeed(0, 0, 0);
	}
};
