#include "views/ProgressBar.h"

void TestUltrasonicActivity_resetLineFlag(){
	LineSensor->reset();
}

class TestUltrasonicActivity : public Activity
{
protected:
	ProgressBar prgBars[4];
	ProgressBar varianceBar[4];
	ProgressBar lightSensorBar;
	
	Button resetLineFlag;

public:
	TestUltrasonicActivity(){
		Activity::Activity();

		ActivityName = String("Test EZ Ultrasonic Sensors");
	
		for (int i = 0; i < 4; i++) {
			prgBars[i].setMax(300);
			prgBars[i].o.x = 25;
			prgBars[i].w = 500;

			varianceBar[i].setMax(60);
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
		
		lightSensorBar = ProgressBar();
		lightSensorBar.o.x = 25;
		lightSensorBar.o.y = prgBars[3].o.y + 2 * prgBars[3].h;

		lightSensorBar.w = 500;
		lightSensorBar.setMax(1);
		lightSensorBar.cProgress = color(255, 30,30);
		addView(&lightSensorBar);

		resetLineFlag = Button("Reset Line");
		resetLineFlag.o = Point(25, 300);
		resetLineFlag.h = 60;
		resetLineFlag.w = 200;
		resetLineFlag.onClick(TestUltrasonicActivity_resetLineFlag);
		addView(&resetLineFlag);

		digitalWrite(pDIST_EN_TOP, HIGH);
		digitalWrite(pDIST_TRIG_TOP, HIGH);
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
				Dist[i]->readDistance(), 
				// Variance
				abs(Dist[i]->getDistance() - average[i].mean) + 10);

			Gaussian g = average[i].process();
			prgBars[i].setValue((int)g.mean);
			varianceBar[i].setValue((int)g.variance);
		}

		lightSensorBar.setValue(LineSensor->getValue());

		Serial.print("c: ");
		Serial.println(millis() - start);

		delay(50);
	};

	void onStop(){
		
	}
};
