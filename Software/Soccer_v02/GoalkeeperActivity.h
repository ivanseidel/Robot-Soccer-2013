#include <views/AngleView.h>
#include <views/TextView.h>
#include <views/ProgressBar.h>
#include <views/CheckBox.h>

#include <math.h>

#define US_FRONT 0
#define US_LEFT 1
#define US_BACK 2
#define US_RIGHT 3

#define BALL_TIME 550

class GoalkeeperActivity : public Activity {
protected:
	CheckBox *toggleBall;
	CheckBox *toggleCompass;

	MoveController *MC;
	CompassPID *compassPID;
	
	long lastLineMillis;
	bool wasInLineMovement;
	
	long lastStuckMillis;
	
	long ballStartMillis;
	int ballChaseTheta;
	
	GaussianAverage average[4];
	int distance[4];
	
	double ballTheta, ballSpeed;
	
	TextView xStatus, yStatus;
	TextView stuckView, angleView;
	TextView stuckMillisView;
	TextView iterationView;
	
	AngleView ballAngleView;
	TextView ballIntensityView;
	
public:
	GoalkeeperActivity(MoveController *_MC) {
		Activity::Activity();
		ActivityName = String("Goalkeeper");
		
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
		
		xStatus.o = Point(10, 250);
		yStatus.o = Point(10, 280);
		stuckView.o = Point(10, 310);
		angleView.o = Point(10, 340);
		stuckMillisView.o = Point(10, 370);
		iterationView.o = Point(10, 400);
		
		addView(&xStatus);
		addView(&yStatus);
		addView(&stuckView);
		addView(&angleView);
		addView(&stuckMillisView);
		addView(&iterationView);
		
		ballAngleView.o = Point(500, 150);
		ballAngleView.w = 75;
		ballIntensityView.o = Point(500, 250);
		
		addView(&ballAngleView);
		addView(&ballIntensityView);

		Threads.add(Ball);
		Threads.add(Compass);
		Threads.add(compassPID);
		
		updateDistances();
		updateDistances();
		updateDistances();
		updateDistances();
		
		lastLineMillis = 0;
		wasInLineMovement = false;
		
		lastStuckMillis = 0;
		
		ballStartMillis = 0;
	}
	
	void onCreate() {
		toggleBall->setChecked(false);
		toggleCompass->setChecked(false);
	}
	
	void updateLineStatus() {
		int minSideDistance = distance[US_LEFT];
		if (distance[US_RIGHT] < minSideDistance)
			minSideDistance = distance[US_RIGHT];
		
		if (LineSensor->readValue() && 
				(minSideDistance <= 40 || distance[US_BACK] <= 25) &&
				millis() - lastLineMillis >= 1000) {
			LineSensor->reset();
			lastLineMillis = millis();
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
		
		if (!wasInLineMovement) {
			wasInLineMovement = true;
			
			int speed = 100;
			int theta = 0;
			
			if (distance[US_LEFT] > distance[US_RIGHT])
				theta = 270 + 33;
			else
				theta = 90 - 33;
			
			lastSpeed = speed;
			lastTheta = theta;
		}
		
		int thetaSpeed = 0;
		if (toggleCompass->isChecked()) {
			thetaSpeed = compassPID->getThetaSpeed();
		}
		
		MC->setSpeed(lastTheta, lastSpeed, thetaSpeed);
	}
	
	void moveAwayFromGoal() {
		int thetaSpeed = 0;
		if (toggleCompass->isChecked()) {
			thetaSpeed = compassPID->getThetaSpeed();
		}
		
		MC->setSpeed(0, 120, thetaSpeed);
	}
	
	int getSignedAngle(int x) {
		while (x > 180)
			x -= 360;
		while (x < -180)
			x += 360;
		return x;
	}
	
	bool nearCenter(int dist, int dist2 = -1) {
		int maxDistance = distance[US_LEFT];
		if (distance[US_RIGHT] > maxDistance)
			maxDistance = distance[US_RIGHT];
		
		if (maxDistance >= dist)
			return false;
		
		if (dist2 != -1) {
			int minDistance = distance[US_LEFT];
			if (distance[US_RIGHT] < minDistance)
				minDistance = distance[US_RIGHT];
			
			if (minDistance < dist2)
				return false;
		}
		
		return true;
	}
	
	bool canChaseBall(int angle, int dist, int dist2 = -1) {
		if (angle > 0 && distance[US_LEFT] <= dist && distance[US_RIGHT] >= dist2)
			return true;
		if (angle < 0 && distance[US_RIGHT] <= dist && distance[US_LEFT] >= dist2)
			return true;
		return false;
	}
	
	void determineBallMovement(double &theta, double &speed) {
		int intensity = Ball->getIntensity();
		int angle = getSignedAngle(Ball->getAngle());
		
		static long lastMillis = 0;
		
		if (millis() - lastMillis < 80)
			return;
		lastMillis = millis();
		
		ballAngleView.setAngle(toRads(angle-90));
		ballIntensityView.setValue(intensity);
		
		theta = speed = 0;
		
		int xSpeed = 0, ySpeed = 0;
		
		angleView.setValue((int) getSignedAngle(Compass->getAngle()));
		
		int targetY = 17;
		if (!nearCenter(90))
			targetY = 40;
		
		bool mustGoBack = false;
		if ((nearCenter(80) && distance[US_BACK] >= 30) || distance[US_BACK] >= 60)
			mustGoBack = true;
		
		if (mustGoBack) {
			ySpeed = -35;
			yStatus.setValue("must go back");
		}
		else if (nearCenter(75, 40) && abs(distance[US_BACK] - targetY) >= 4 && abs(compassPID->getError()) <= 15) {
			if (distance[US_BACK] > targetY) {
				ySpeed = -35;
				yStatus.setValue("y too far");
			}
			else {
				ySpeed = 35;
				yStatus.setValue("y too close");
			}
		}
		else
			yStatus.setValue("y ok");
		
		if (intensity >= 3 && nearCenter(80, 40)) {
			if (abs(angle) > 45 && canChaseBall(angle, 75, 40)) {
				if (angle > 0) {
					xSpeed = 110;
					xStatus.setValue("x to the right for ball fast");
				}
				else {
					xSpeed = -110;
					xStatus.setValue("x to the left for ball fast");
				}
			}
			else if (angle >= 3 && canChaseBall(angle, 75, 40)) {
				if (angle > 0) {
					xSpeed = 90;
					xStatus.setValue("x to the right for ball slow");
				}
				else {
					xSpeed = -90;
					xStatus.setValue("x to the left for ball slow");
				}
			}
			else {
				xSpeed = 0;
				xStatus.setValue("ball ok");
			}
		}
		else {
			int maxSideDistance = distance[US_LEFT];
			int mult = 1;
			if (distance[US_RIGHT] > distance[US_LEFT]) {
				maxSideDistance = distance[US_RIGHT];
				mult = -1;
			}
			
			if (abs(maxSideDistance - 65) > 8) {
				if (maxSideDistance > 55) {
					xSpeed = -mult * 80;
					
					if (mult == 1)
						xStatus.setValue("x to the left for center");
					else
						xStatus.setValue("x to the right for center");
				}
				else {
					xSpeed = mult * 80;
					
					if (mult == 1)
						xStatus.setValue("x to the right for center else");
					else
						xStatus.setValue("x to the left for center else");
				}
			}
			else {
				xStatus.setValue("x ok no ball");
			}
		}
		
		theta = 180.0 * atan2(ySpeed, xSpeed) / M_PI;
		theta -= 90; //fix offset
		theta = -getSignedAngle(theta); //fix cw/ccw
		
		speed = hypot(ySpeed, xSpeed);
	}
	
	void updateBallStatus() {
		int intensity = Ball->getIntensity();
		int angle = getSignedAngle(Ball->getAngle());
		
		if (distance[US_BACK] <= 25 && abs(angle) <= 50 && intensity >= 110 && millis() - ballStartMillis >= 4*BALL_TIME) {
			ballStartMillis = millis();
			ballChaseTheta = angle;
		}
	}
	
	void ballMovement() {
		int theta = 0, speed = 0, thetaSpeed = 0;
		
		if (millis() - ballStartMillis < BALL_TIME)
			theta = 1.4*ballChaseTheta;
		else
			theta = getSignedAngle(1.4*ballChaseTheta + 180);
		
		speed = 120;
		
		if (millis() - ballSpeed >= BALL_TIME)
			speed = 100;
		
		if (false && toggleCompass->isChecked()) {
			thetaSpeed = compassPID->getThetaSpeed();
		}
		
		MC->setSpeed(theta, speed, thetaSpeed);
	}
	
	void onLoop() {
		double theta = 0, speed = 0, thetaSpeed = 0;
		static long lastMillis = 0;
		
		iterationView.setValue((int) (millis() - lastMillis));
		lastMillis = millis();
	
		updateDistances();
		updateLineStatus();
		updateBallStatus();
		
		if (!toggleCompass->isChecked()) {
			MC->setSpeed(0, 0, 0);
			return;
		}
		
		if (millis() - ballStartMillis < 2*BALL_TIME) {
			ballMovement();
			return;
		}
		
		if (millis() - lastLineMillis < 700) {
			moveAwayFromLine();
			return;
		}
		else {
			wasInLineMovement = false;
		}
		
		determineBallMovement(ballTheta, ballSpeed);
		if (toggleBall->isChecked()) {
			theta = ballTheta;
			speed = ballSpeed;
		}
		
		if (toggleCompass->isChecked()) {
			thetaSpeed = compassPID->getThetaSpeed();
		}
	
		MC->setSpeed(theta, speed, thetaSpeed);
	}
	
	virtual bool enablePause(){
		return true;
	}

	virtual void onPause(){
		MC->setSpeed(0, 0, 0);
	}
};