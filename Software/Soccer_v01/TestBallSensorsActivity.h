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
		
		angle.o = Point(300, 80);
		angle.w = 80;
		
		prgBar.o = Point(100, 200);
		prgBar.w = 400;
		prgBar.setMax(255);
		
		addView(&angle);
		addView(&prgBar);
	}
	
	void onLoop() {
		if (BALL_USART.available()) {
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
		}
	}
};