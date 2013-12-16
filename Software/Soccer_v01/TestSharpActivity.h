#include "views/ProgressBar.h"

class TestSharpActivity : public Activity
{
protected:
	ProgressBar prgBars[4];
	SharpLongMod *sharp[4];
	
public:
	TestSharpActivity(){
		Activity::Activity();

		ActivityName = String("Test Sharp IR Sensors");
		
		sharp[0] = new SharpLong(pDIST_1);
		sharp[1] = new SharpLong(pDIST_2);
		sharp[2] = new SharpLong(pDIST_3);
		sharp[3] = new SharpLong(pDIST_4);
		
		for (int i = 0; i < 4; i++) {
			prgBars[i].setMax(150);
			prgBars[i].o.x = 50;
			prgBars[i].w = 300;
			
			if (i == 0)
				prgBars[i].o.y = 50;
			else
				prgBars[i].o.y = prgBars[i - 1].o.y + 2 * prgBars[i - 1].h;
			
			addView(&prgBars[i]);
		}
		
		digitalWrite(pDIST_EN_BOT, HIGH);
	}

	void onResume(){
		
	}

	void onLoop(){
		for (int i = 0; i < 4; i++) {
			//prgBars[i].setValue(sharp[i]->readDistance());
			prgBars[i].setValue(sharp[i]->read());
		}
	};

	void onStop(){
		
	}
};
