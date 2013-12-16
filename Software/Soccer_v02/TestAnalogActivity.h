#include "views/ProgressBar.h"

class TestAnalogActivity : public Activity
{
protected:
	ProgressBar prgBars[4];
	ProgressBar varianceBar[4];
	
	

public:
	TestAnalogActivity(){
		Activity::Activity();

		ActivityName = String("Test Analog Sensors");
	
		for (int i = 0; i < 4; i++) {
			prgBars[i].setMax(1023);
			prgBars[i].o.x = 25;
			prgBars[i].w = 500;

			varianceBar[i].setMax(200);
			varianceBar[i].o.x = 600;
			varianceBar[i].w = 150;
			varianceBar[i].cProgress = color(255,30,30);
			
			if (i == 0){

				prgBars[i].o.y = 50;
				
			}else{
				prgBars[i].o.y = prgBars[i - 1].o.y + 2 * prgBars[i - 1].h;
			}

			varianceBar[i].o.y = prgBars[i].o.y;
			
			addView(&prgBars[i]);
			addView(&varianceBar[i]);
		}
		
		digitalWrite(pDIST_EN_BOT, HIGH);
	}

	void onResume(){
		
	}

	GaussianAverage average[4];

	void onLoop(){

		// Calculate time needed to process
		long start = millis();

		for (int i = 0; i < 4; i++){

			average[i] += Gaussian(
				// Mean
				analogPins[i]->read(), 
				// Variance
				abs(analogPins[i]->read() - average[i].mean) + 10);

			Gaussian g = average[i].process();
			prgBars[i].setValue((int)g.mean);
			varianceBar[i].setValue((int)g.variance);
		}

		Serial.print("c: ");
		Serial.println(millis() - start);

		delay(50);
	};

	void onStop(){
		
	}
};
