#include "views/TextView.h"

#define compassAddress (0x42 >> 1)
#define TIME_LIMIT 	100

#include <Wire.h>

class CalibrateActivity : public Activity {
protected:
	MoveController *MC;
	
	TextView starting, done;
	
public:
	CalibrateActivity(MoveController *_MC) {
		Activity::Activity();
		ActivityName = "Calibrate Compass";
		
		MC = _MC;
		
		starting.o = Point(50, 50);
		starting.setValue(String("Starting..."));
		addView(&starting);
		
		done.o = Point(50, 150);
		done.setValue(String("Done."));
		
		Wire.begin();
	}
	
	int compassCalibrate() {
		int compassValue;
		long timeLimit_InitialTime;
  
		Wire.beginTransmission(compassAddress);
		Wire.write('C');
		Wire.endTransmission();
		delay(10000);
		delay(5000);
		delay(4999);
		Wire.beginTransmission(compassAddress);
		Wire.write('E');
		Wire.endTransmission();	  
	}
	
	boolean compassCheck()
{
  long timeLimit_InitialTime;

  Wire.beginTransmission(compassAddress);
  Wire.write('A');
  Wire.endTransmission();

  Wire.requestFrom(compassAddress, 2);
  
  timeLimit_InitialTime = millis();

  while(Wire.available() < 2)
  {
     if((millis() - timeLimit_InitialTime) > TIME_LIMIT)
     {
       return false;
     }
  }
  
  Wire.read();
  Wire.read();
  
  return true;
}
	
	void onLoop() {
		compassCalibrate();
		
		addView(&done);
		done.setValue(String("Done."), true);
		
		if (!compassCheck()) {
			TextView error;
			error.o = Point(50, 200);
			addView(&error);
			error.setValue(String("Error!"), true);
		}
		
		while (true);
	}
};