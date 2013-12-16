class TestCompassActivity: public Activity
{
protected:
	AngleView yprViews;
	
	bool isCalibrating;

public:
	
	static TestCompassActivity *current;

	Button startCalibrationBtn;
	Button stopCalibrationBtn;

	TestCompassActivity();

	void onCreate(){
		isCalibrating = false;

		startCalibrationBtn.cBg = color(20,200,20);
		startCalibrationBtn.cBgHit = color(20,255,20);
		startCalibrationBtn.invalidate();

		stopCalibrationBtn.cBg = color(200,200,200);
		stopCalibrationBtn.cBgHit = color(255,20,20);
		stopCalibrationBtn.invalidate();
	}

	void onResume(){
		// Compass->setQueryMode();
		current = this;
	}

	void startCalibration(){
		if(isCalibrating)
			return;

		beep();

		Compass->startCalibration();
		isCalibrating = true;

		startCalibrationBtn.cBg = color(200,200,200);
		startCalibrationBtn.cBgHit = color(20,255,20);
		startCalibrationBtn.invalidate();

		stopCalibrationBtn.cBg = color(200,20,20);
		stopCalibrationBtn.cBgHit = color(255,20,20);
		stopCalibrationBtn.invalidate();
	}

	void stopCalibration(){
		if(!isCalibrating)
			return;

		beep();

		Compass->finishCalibration();
		isCalibrating = false;

		startCalibrationBtn.cBg = color(20,200,20);
		startCalibrationBtn.cBgHit = color(20,255,20);
		startCalibrationBtn.invalidate();

		stopCalibrationBtn.cBg = color(200,200,200);
		stopCalibrationBtn.cBgHit = color(255,20,20);
		stopCalibrationBtn.invalidate();
	}

	void onLoop(){
		if(!isCalibrating)
			yprViews.setAngle(toRads(-Compass->readAngle()));

		delay(50);
	}

	void onStop(){
		if(isCalibrating){
			stopCalibration();
		}
	}
};

void TestCompassActivity_startCalibration(){
	Serial.println("startCalibration");
	if(!TestCompassActivity::current)
		return;

	TestCompassActivity::current->startCalibration();
}

void TestCompassActivity_stopCalibration(){
	Serial.println("stopCalibration");
	if(!TestCompassActivity::current)
		return;

	TestCompassActivity::current->stopCalibration();
}

TestCompassActivity::TestCompassActivity(){
	Activity::Activity();

	ActivityName = String("Test Compass Sensor");

	yprViews = AngleView();
	yprViews.o = Point(400, 220);
	yprViews.w = 200;
	addView(&yprViews);


	startCalibrationBtn.name = String("Calibrate");
	startCalibrationBtn.o = Point(10,10);
	startCalibrationBtn.w = 180;
	startCalibrationBtn.h = 60;
	startCalibrationBtn.onClick(TestCompassActivity_startCalibration);
	addView(&startCalibrationBtn);

	stopCalibrationBtn.name = String("Stop!");
	stopCalibrationBtn.o = Point(10,90);
	stopCalibrationBtn.w = 180;
	stopCalibrationBtn.h = 60;
	stopCalibrationBtn.onClick(TestCompassActivity_stopCalibration);
	addView(&stopCalibrationBtn);

	isCalibrating = false;
}
TestCompassActivity* TestCompassActivity::current = false;