#include "views/TextView.h"

#include "SoccerLocator.h"
#include "sensors/EZUltrasonic.h"

class TestSoccerLocatorActivity : public Activity {
protected:
	TextView distViews[4], trustedViews[4], posXView, posYView;
	SoccerLocator *locator;
	
public:
	TestSoccerLocatorActivity() {
		Activity::Activity();
		ActivityName = String("Test Soccer Locator");
		
		DistanceInterface *sensors[4] = { new EZUltrasonic(pDIST_6), 
										  new EZUltrasonic(pDIST_7), 
									      new EZUltrasonic(pDIST_8), 
										  new EZUltrasonic(pDIST_9) };
		
		digitalWrite(pDIST_EN_TOP, HIGH);
		digitalWrite(pDIST_TRIG_TOP, HIGH);
		
		locator = new SoccerLocator(0, sensors);
		Threads.add(locator);
		
		for (int i = 0; i < 4; i++) {
			distViews[i].o = Point(50 + 100 * i, 100);
			trustedViews[i].o = Point(50 + 100 * i, 150);
			addView(&distViews[i]);
			addView(&trustedViews[i]);
		}
		
		posXView.o = Point(50, 200);
		addView(&posXView);
		
		posYView.o = Point(150, 200);
		addView(&posYView);
	}
	
	void onLoop() {
		static long lastMillis = millis();
		
		Threads.run();
		
		if (millis() - lastMillis >= 100) {
			for (int i = 0; i < 4; i++) {
				distViews[i].setValue(locator->getDistance(i));
				trustedViews[i].setValue(locator->isTrusted(i));
			}
			posXView.setValue(locator->posX());
			posYView.setValue(locator->posY());
			
			lastMillis = millis();
		}
	}
};