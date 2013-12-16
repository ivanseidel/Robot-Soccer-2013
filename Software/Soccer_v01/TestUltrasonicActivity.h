#include "views/ProgressBar.h"

class TestUltrasonicActivity : public Activity
{
protected:
	ProgressBar prgBars[4];
	EZUltrasonic *us[4];
	
public:
	TestUltrasonicActivity(){
		Activity::Activity();

		ActivityName = String("Test EZ Ultrasonic Sensors");
		
		us[0] = new EZUltrasonic(pDIST_6);
		us[1] = new EZUltrasonic(pDIST_7);
		us[2] = new EZUltrasonic(pDIST_8);
		us[3] = new EZUltrasonic(pDIST_9);
		
		for (int i = 0; i < 4; i++) {
			prgBars[i].setMax(300);
			prgBars[i].o.x = 50;
			prgBars[i].w = 500;
			
			if (i == 0)
				prgBars[i].o.y = 50;
			else
				prgBars[i].o.y = prgBars[i - 1].o.y + 2 * prgBars[i - 1].h;
			
			addView(&prgBars[i]);
		}
		
		digitalWrite(pDIST_EN_TOP, HIGH);
		digitalWrite(pDIST_TRIG_TOP, HIGH);
	}

	void onResume(){
		
	}

	void onLoop(){
		for (int i = 0; i < 4; i++)
			prgBars[i].setValue(us[i]->readDistance());
	};

	void onStop(){
		
	}
};
