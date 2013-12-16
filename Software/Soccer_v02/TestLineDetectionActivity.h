#include "views/ProgressBar.h"

#define LINE_MIN_VARIANCE 10
#define LINE_MIN_OFFSET 80

class TestLineDetectionActivity : public Activity
{
protected:
	ProgressBar prgBars[4];
	ProgressBar varianceBar[4];
	ProgressBar simpleAverageBar[4];
	
	int simpleAverageAll[4];
	bool enabled[4];
	
	bool isActive;
	
	MoveController *MC;

public:
	TestLineDetectionActivity(bool _enabled[4], MoveController *_MC){
		Activity::Activity();
		
		MC = _MC;
		
		isActive = true;
		
		ActivityName = String("Test Line Detection");
	
		for (int i = 0; i < 4; i++) {
			enabled[i] = _enabled[i];
		
			prgBars[i].setMax(1023);
			prgBars[i].o.x = 25;
			prgBars[i].w = 500;

			varianceBar[i].setMax(200);
			varianceBar[i].o.x = 600;
			varianceBar[i].w = 150;
			varianceBar[i].cProgress = color(255,30,30);
			
			simpleAverageBar[i].setMax(1023);
			simpleAverageBar[i].o.x = 25;
			simpleAverageBar[i].w = 500;
			
			if (i == 0){

				prgBars[i].o.y = 50;
				simpleAverageBar[i].o.y = 50 + 8 * prgBars[0].h;
				
			}else{
				prgBars[i].o.y = prgBars[i - 1].o.y + 2 * prgBars[i - 1].h;
				simpleAverageBar[i].o.y = simpleAverageBar[i - 1].o.y + 2 * simpleAverageBar[i - 1].h;
			}

			varianceBar[i].o.y = prgBars[i].o.y;
			
			addView(&prgBars[i]);
			addView(&varianceBar[i]);
			addView(&simpleAverageBar[i]);
		}
		
		digitalWrite(pDIST_EN_BOT, HIGH);
		
		for (int i = 0; i < 4; i++)
			simpleAverageAll[i] = analogPins[i]->read();
	}

	void onResume(){
		isActive = true;
	}

	GaussianAverage average[4];

	void onLoop(){
		/*for (int i = 0; i < 4; i++){
			int reading = analogPins[i]->read();

			average[i] += Gaussian(
				// Mean
				reading, 
				// Variance
				abs(reading - average[i].mean) + 10);

			Gaussian g = average[i].process();
			prgBars[i].setValue((int)g.mean);
			varianceBar[i].setValue((int)g.variance);
			
			double cOld = .99;
			double cNew = 1 - cOld;
			
			simpleAverageAll[i] = (int) (cOld * (double) simpleAverageAll[i] + cNew * (double) reading);
			simpleAverageBar[i].setValue(simpleAverageAll[i]);
			
			if (enabled[i]) {
				if ((int) g.mean - simpleAverageAll[i] >= LINE_MIN_OFFSET && (int) g.variance >= LINE_MIN_VARIANCE)
					beep(5);
			}
		}

		delay(50);*/
		
		if (isActive) {
			MC->setSpeed(180, 70);
		
			for (int i = 0; i < 4; i++) if (enabled[i]) {
				double cOld = .95;
				double cNew = 1 - cOld;
				
				int reading = analogPins[i]->read();
				
				simpleAverageAll[i] = (int) (cOld * (double) simpleAverageAll[i] + cNew * (double) reading);
				simpleAverageBar[i].setValue(reading - simpleAverageAll[i]);
				
				if (reading - simpleAverageAll[i] >= LINE_MIN_OFFSET)
					isActive = false;
			}
		}
		else {
			MC->setSpeed(0, 0);
		}
	}

	void onStop(){
		isActive = false;
	}
};
