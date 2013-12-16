/*
	Class that helps interrupt driven pins that reads REALLY fast
	changes. This class has common methods to reset the flag, and
	also to set.

	This class is SPECIFICLY made for the Light sensor on the robot
*/

bool LightSensor_STATE = false;
void LightSensor_onHIGH(){
	LightSensor_STATE = true;
	// Serial.println("LINE");
}

class LightSensor: public DigitalIn{
public:
	LightSensor(int _inPin, bool invert = false): DigitalIn(_inPin, invert){
		attachInterrupt(_inPin, LightSensor_onHIGH, RISING );
	}

	virtual bool readValue(){
		return LightSensor_STATE;
	}

	virtual bool getValue(){
		return LightSensor_STATE;
	}

	void reset(){
		LightSensor_STATE = false;
	}


};

