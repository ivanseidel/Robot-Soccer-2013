#include <views/AngleView.h>
#include <views/TextView.h>
#include <views/ProgressBar.h>
#include <views/CheckBox.h>

#include <math.h>

#define US_FRONT 0
#define US_LEFT 1
#define US_BACK 2
#define US_RIGHT 3

#define LINE_LONG_MILLIS 500
#define LINE_SHORT_MILLIS 500

#define SQR(x) ((x) * (x))

#define KICK_TIME 100

class ForwardActivity : public Activity {
protected:
	CheckBox *toggleBall;
	CheckBox *toggleCompass;

	MoveController *MC;
	CompassPID *compassPID;
	
	long lastLineMillis;
	bool lineCloseTo[4];
	bool wasInLineMovement;
	long awayLineMillis;
	int frequentLineCount;
	
	double ballTheta, ballSpeed;
	
	GaussianAverage average[4];
	int distance[4];
	int maxDistClose[4];
	
	TextView closeToView[4];
	TextView thetaView;
	TextView ballView;
	TextView angleView;
	
	GaussianAverage *thetaAverage, *speedAverage;
	
	GaussianAverage xAverage, yAverage;
	
	long kickMillis;
	int kickSide;
	long stopMillis;
	int kickFactor;
	
public:
	ForwardActivity(MoveController *_MC) {
		Activity::Activity();
		ActivityName = String("Forward Player");
		
		MC = _MC;
		compassPID = new CompassPID();
		
		toggleBall = new CheckBox("Ball");
		toggleBall->o = Point(10, 10);
		toggleBall->w = toggleBall->h = 150;
		
		toggleCompass = new CheckBox("Compass");
		toggleCompass->o = Point(200, 10);
		toggleCompass->w = toggleCompass->h = 150;
		
		addView(toggleBall);
		addView(toggleCompass);

		Threads.add(Ball);
		Threads.add(Compass);
		Threads.add(compassPID);
		
		updateDistances();
		updateDistances();
		updateDistances();
		updateDistances();
		
		for (int i = 0; i < 4; i++) {
			closeToView[i].o = Point(10 + i * 20, 200);
			addView(&closeToView[i]);
		}
		
		thetaView.o = Point(10, 230);
		addView(&thetaView);
		
		ballView.o = Point(10, 260);
		addView(&ballView);
		
		angleView.o = Point(30, 300);
		addView(&angleView);
		
		lastLineMillis = 0;
		wasInLineMovement = false;
		
		ballTheta = ballSpeed = 0;
		
		thetaAverage = new GaussianAverage(1);
		speedAverage = new GaussianAverage(1);
		
		xAverage = GaussianAverage(4);
		yAverage = GaussianAverage(4);
		
		maxDistClose[US_FRONT] = 26;
		maxDistClose[US_LEFT] = 26;
		maxDistClose[US_BACK] = 26;
		maxDistClose[US_RIGHT] = 26;
		
		awayLineMillis = 0;
		frequentLineCount = 0;
		kickMillis = 0;
		kickSide = US_LEFT;
		stopMillis = 0;
		kickFactor = 1;
	}
	
	void onCreate() {
		toggleBall->setChecked(false);
		toggleCompass->setChecked(false);
	}
	
	void updateLineStatus() {
		if (millis() - lastLineMillis < awayLineMillis)
			return;
		
		int count = 0;
		for (int i = 0; i < 4; i++) {
 			if (distance[i] <= maxDistClose[i]) {
				lineCloseTo[i] = true;
				count++;
			}
			else
				lineCloseTo[i] = false;
			
			closeToView[i].setValue(lineCloseTo[i]);
		}
	
		if (LineSensor->readValue() && count > 0 && millis() - lastLineMillis >= 500) {
			bool necessary[4] = { 0 };
			
			double myY = yAverage.process().mean;
			double myX = xAverage.process().mean;
			
			double averageTheta = atan2(myY, myX);
			int heading = getSignedAngle(toDegs(averageTheta));
			
			if (-90 <= heading && heading <= 90)
				necessary[US_FRONT] = true;
			else
				necessary[US_BACK] = true;
			
			if (0 <= heading && heading <= 180)
				necessary[US_RIGHT] = true;
			else
				necessary[US_LEFT] = true;
			
			int cntNecessary = 0;
			for (int i = 0; i < 4; i++) {
				if (necessary[i] && lineCloseTo[i])
					cntNecessary++;
			}
			
			myY = yAverage.process().mean;
			myX = xAverage.process().mean;
			
			/*if (myY > 0 && !lineCloseTo[US_FRONT])
				myY = 0;
			if (myY < 0 && !lineCloseTo[US_BACK])
				myY = 0;
			if (myX > 0 && !lineCloseTo[US_RIGHT])
				myX = 0;
			if (myX < 0 && !lineCloseTo[US_LEFT])
				myX = 0;*/
			
			averageTheta = atan2(myY, myX);
			heading = getSignedAngle(toDegs(averageTheta));
			
			if (cntNecessary) {
				if (millis() - lastLineMillis < 1500) {
					frequentLineCount++;
					stopMillis = millis();
				}
				else
					frequentLineCount = 0;
			
				LineSensor->reset();
				lastLineMillis = millis();
				
				if (lineCloseTo[US_FRONT] || lineCloseTo[US_BACK])
					awayLineMillis = LINE_LONG_MILLIS;
				else
					awayLineMillis = LINE_SHORT_MILLIS;
			}
		}
	}
	
	void updateDistances() {
		static long lastMillis = 0;
		
		if (millis() - lastMillis < 80)
			return;
		lastMillis = millis();
		
		for (int i = 0; i < 4; i++){
			average[i] += Gaussian(
				// Mean
				Dist[i]->readDistance(), 
				// Variance
				abs(Dist[i]->getDistance() - average[i].mean) + 10);

			Gaussian g = average[i].process();
			distance[i] = g.mean;
		}
	}
	
	void moveAwayFromLine() {
		static int lastTheta = 0;
		static int lastSpeed = 0;
		
		double averageTheta = atan2(yAverage.process().mean, xAverage.process().mean);
		double averageSpeed = sqrt(SQR(yAverage.process().mean) + SQR(xAverage.process().mean));
		
		averageTheta = toDegs(averageTheta) + 180.0;
		averageSpeed *= 1.5;
		
		MC->setSpeed(averageTheta, averageSpeed, 0);
		return;
		
		//-------------------------------------------------------------------------
		
		if (!wasInLineMovement || true) {
			wasInLineMovement = true;
			
			int speed = 100;
			int theta = 180;
			
			long delta = millis() - lastLineMillis;
			
			if (lineCloseTo[US_FRONT]) {
				//theta = distance[US_LEFT] < distance[US_RIGHT] ? 90 + 0*33 : 270 - 0*33;
				
				//if (delta < 2 * awayLineMillis / 3)
					theta = 180;
				/*else {
					if (distance[US_LEFT] < distance[US_RIGHT])
						theta = 90;
					else
						theta = 270;
				}*/
			}
			else if (lineCloseTo[US_BACK]) {
				//theta = distance[US_LEFT] < distance[US_RIGHT] ? 90 - 0*33 : 270 + 0*33;
				
				//if (delta < 2 * awayLineMillis / 3)
					theta = 0;
				/*else {
					if (distance[US_LEFT] < distance[US_RIGHT])
						theta = 90;
					else
						theta = 270;
				}*/
			}
			else if (lineCloseTo[US_LEFT])
				theta = 90;
			else if (lineCloseTo[US_RIGHT])
				theta = 270;
			
			/*if (lineCloseTo[US_LEFT]) {
				if (distance[US_FRONT] > distance[US_BACK])
					theta = 90 - 33;
				else
					theta = 90 + 33;
				
				if (distance[US_FRONT] >= 30 && distance[US_BACK] >= 30)
					theta = 90;
			}
			else if (lineCloseTo[US_RIGHT]) {
				if (distance[US_FRONT] > distance[US_BACK])
					theta = 270 + 33;
				else
					theta = 270 - 33;
				
				if (distance[US_FRONT] >= 30 && distance[US_BACK] >= 30)
					theta = 270;
			}
			else if (lineCloseTo[US_FRONT]) {
				if (distance[US_LEFT] > distance[US_RIGHT])
					theta = 270 - 33;
				else
					theta = 90 + 33;
			}
			else if (lineCloseTo[US_BACK]) {
				if (distance[US_LEFT] > distance[US_RIGHT])
					theta = 270 + 33;
				else
					theta = 90 - 33;
			}*/
			
			lastTheta = theta;
			lastSpeed = speed;
			
			thetaView.setValue(lastTheta);
		}
		
		int thetaSpeed = 0;
		if (toggleCompass->isChecked()) {
			thetaSpeed = compassPID->getThetaSpeed();
		}
		
		MC->setSpeed(lastTheta, lastSpeed, thetaSpeed);
	}
	
	int getSignedAngle(int x) {
		while (x > 180)
			x -= 360;
		while (x < -180)
			x += 360;
		return x;
	}
	
	void determineBallMovement(double &theta, double &speed) {
		int intensity = Ball->getIntensity();
		int angle = getSignedAngle(Ball->getAngle());
		
		static long lastMillis = 0;
		
		if (millis() - lastMillis < 100)
			return;
		lastMillis = millis();
		
		theta = speed = 0;
		
		if (-60 <= angle && angle < -20)
			angle -= 30;
		
		if (intensity >= 80) {
			if (abs(angle) <= 20) {
				theta = angle;
				speed = 50;
				ballView.setValue("near 20");
			}
			else if (abs(angle) <= 45) {
				speed = 50;
				if (angle > 0) {
					theta = 2.5 * angle;
					ballView.setValue("near 45 positive");
				}
				else {
					theta = 2.5 * angle;
					ballView.setValue("near 45 negative");
				}
			}
			else if (abs(angle) <= 90) {
				speed = 60;
				if (angle > 0) {
					theta = 2.0 * angle;
					ballView.setValue("near 90 positive");
				}
				else {
					theta = 2.0 * angle;
					ballView.setValue("near 90 negative");
				}
			}
			else if (abs(angle) <= 120) {
				speed = 60;
				if (angle > 0) {
					theta = 180;
					ballView.setValue("near 120 positive");
				}
				else {
					theta = 180;
					ballView.setValue("near 120 negative");
				}
			}
			else if (abs(angle) <= 150) {
				speed = 60;
				if (angle > 0) {
					theta = -angle;
					ballView.setValue("near 150 positive");
				}
				else {
					theta = -angle;
					ballView.setValue("near 150 negative");
				}
			}
			else {
				speed = 60;
				if (angle > 0) {
					theta = 270;
					ballView.setValue("near behind positive");
				}
				else {
					theta = 90;
					ballView.setValue("near behind negative");
				}
			}
		}
		else {
			int angles[4] = { 90 - 33, 90 + 33, 270 - 33, 270 + 33 };
			int minDiff = 1000;
			for (int i = 0; i < 4; i++) {
				int diff = abs(getSignedAngle(angles[i] - angle));
				
				if (diff < minDiff) {
					minDiff = diff;
					theta = angles[i];
				}
			}
			
			theta = angle;
			
			if (theta >= 150)
				theta = 150;
			if (theta <= -150)
				theta = -150;
			
			ballView.setValue((int) angle);
			
			if (intensity >= 100)
				speed = 60;
			else
				speed = 60;
		}
	}
	
	void updateTargetAngle() {
		if (distance[US_FRONT] <= 65) {
			if (distance[US_LEFT] < distance[US_RIGHT])
				compassPID->targetDelta = 20 * 0;
			else
				compassPID->targetDelta = -20 * 0;
		}
		else
			compassPID->targetDelta = 0;
	}
	
	void updateXYAverages() {
		static long lastMillis = 0;
		
		if (millis() - lastMillis < 50)
			return;
		
		double theta = thetaAverage->process().mean;
		double speed = speedAverage->process().mean;
		
		double x = speed * cos(toRads(theta));
		double y = speed * sin(toRads(theta));
		
		xAverage.add(x);
		yAverage.add(y);
	}
	
	void updateKickConditions() {
		static long lastFarMillis = 0;
		
		if (Ball->getIntensity() < 100 || abs(getSignedAngle(Ball->getAngle())) > 20)
			lastFarMillis = millis();
		
		if (millis() - lastFarMillis >= 500 && millis() - kickMillis >= 3000) {
			kickMillis = millis();
			
			if (abs(distance[US_LEFT] - distance[US_RIGHT]) < 30) {
				kickSide = US_FRONT;
				kickFactor = 3;
			}
			else if (distance[US_LEFT] < distance[US_RIGHT]) {
				kickSide = US_RIGHT;
				kickFactor = 1;
			}
			else {
				kickSide = US_LEFT;
				kickFactor = 1;
			}
		}
	}
	
	void kick(bool invert = false) {
		if (invert) {
			if (kickSide == US_FRONT)
				MC->setSpeed(0, 0, 0);
			else
				MC->setSpeed(0, 0, compassPID->getThetaSpeed());
		}
		else {
			if (kickSide == US_FRONT) {
				int mult = invert ? -1 : 1;
				MC->setSpeed(0, mult * 255, 0);
				return;
			}
		
			int left = US_LEFT;
			if (invert)
				left = US_RIGHT;
			
			if (kickSide == left)
				MC->setSpeed(0, 0, 255);
			else
				MC->setSpeed(0, 0, -255);	
		}
	}
	
	void onLoop() {
		double theta = 0, speed = 0, thetaSpeed = 0;
	
		updateDistances();
		updateLineStatus();
		
		updateTargetAngle();
		
		updateKickConditions();
		
		if (!toggleCompass->isChecked()) {
			MC->setSpeed(0, 0, 0);
			return;
		}
		
		if (frequentLineCount >= 3) {
			if (millis() - stopMillis < 3000) {
				MC->setSpeed(0, 0, 0);
				return;
			}
			else
				frequentLineCount = 0;
		}
		
		if (millis() - lastLineMillis < awayLineMillis) {
			moveAwayFromLine();
			return;
		}
		else {
			wasInLineMovement = false;
			thetaView.setValue("");
		}
		
		if (millis() - kickMillis < KICK_TIME * kickFactor) {
			kick();
			return;
		}
		else if (millis() - kickMillis < 4 * KICK_TIME) {
			kick(true);
			return;
		}
		
		determineBallMovement(ballTheta, ballSpeed);
		if (toggleBall->isChecked()) {
			theta = ballTheta;
			speed = ballSpeed;
		}
		
		angleView.setValue((int) Compass->getAngle());
		if (toggleCompass->isChecked()) {
			thetaSpeed = compassPID->getThetaSpeed();
		}
		
		double variance = 1.0 / Ball->getIntensity();
		thetaAverage->add(Gaussian(theta, variance));
		speedAverage->add(Gaussian(speed, variance));
		
		updateXYAverages();
	
		MC->setSpeed(thetaAverage->process().mean, speedAverage->process().mean, thetaSpeed);
	}
	
	virtual bool enablePause(){
		return true;
	}

	virtual void onPause(){
		MC->setSpeed(0, 0, 0);
	}
};