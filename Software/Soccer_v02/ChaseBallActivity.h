#include <views/AngleView.h>
#include <views/ProgressBar.h>

#include <math.h>

class ChaseBallActivity : public Activity {
protected:
	AngleView angle;
	ProgressBar prgBar;
	
	MoveController *MC;
	BallRadar *Ball;
	
public:
	ChaseBallActivity(MoveController *_MC, BallRadar *_Ball) {
		Activity::Activity();
		ActivityName = String("Chase Ball");
		
		MC = _MC;
		Ball = _Ball;
		
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
	
	void onLoop() {
		prgBar.setValue(Ball->getIntensity());
		angle.setAngle(toRads(Ball->getAngle()));
		
		MC->setSpeed(Ball->getAngle(), constrain(Ball->getIntensity() * 2, 50, 255), 0);
	}
};