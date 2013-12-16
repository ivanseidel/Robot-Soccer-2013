#include <views/AngleView.h>
#include <views/ProgressBar.h>

#include <math.h>

class TestBallSensorsActivity : public Activity {
protected:
	AngleView angle;
	ProgressBar prgBar;
	
public:
	TestBallSensorsActivity() {
		Activity::Activity();
		ActivityName = String("Test Ball Sensors");
		
		angle.o = Point(400, 220);
		angle.w = 200;
		
		prgBar.o = Point(20, 20);
		prgBar.h = 400;
		prgBar.w = 20;
		prgBar.setMax(255);
		prgBar.type = ProgressBar::LINEAR_UP;
		
		addView(&angle);
		addView(&prgBar);

		Threads.add(Ball);
	}

	long lastMillis;
	
	void onLoop() {
		if(!Ball->newReading)
			return;

		Ball->newReading = false;
		long nowMillis = millis();
		long delta = nowMillis - lastMillis;
		lastMillis = nowMillis;
		/*if (BALL_USART.available()) {
			char data = BALL_USART.read();
			Serial.print("DATA: ");
			Serial.println(data, DEC);
			//odd -> intensity
			if (data & 1) {
				prgBar.setValue(data);
			}
			else { //even -> shifted direction
				int direction = data >> 1;
				if (direction == 80)
					direction = 0;
				direction = map(direction, 0, 31, 0, 360);
				
				//Transform to match robot, 0 degree -> front
				direction -= 90;
				if (direction <= 0)
					direction += 360;
				
				double ballAngle = (double) direction * M_PI / 180.0;
				angle.setAngle(ballAngle);
				
				Serial.println(toDegs(ballAngle));
			}
		}*/
		prgBar.setValue(Ball->getIntensity());
		angle.setAngle(toRads(Ball->getAngle()-90.0));



		PC.print("Angle: ");
		PC.print(Ball->getAngle());
		PC.print(" | Delta: ");
		PC.println(delta);

		// delay(50);
	}
};