// ------ ECONOMIZE MEMORY and PROGRAM -------
// Disable Definitions (Make code lighter)

// #define DISABLE_ZYGOTE	1

// #define DISABLE_VICTIM	1
// #define DISABLE_SENSORS	1
#define DISABLE_IMU		1
// #define DISABLE_MOTION	1
// #define DISABLE_LCD		1 	// Also disables Touch
								// Disabling LCD will cause WEIRD things
// --------------------------------------------


// Motors incluedes
#ifndef DISABLE_MOTION
	/*#include "MoveController.h"
	#include ".h"*/ // Driver include
#endif

// General peripheral includes
#include <DueTimer.h>

#include <ArdUI.h>
#include <View.h>

// LCD stuff
#ifndef DISABLE_LCD
	#include <UTouch.h>
	#include <UTFT.h>
#endif

//#include "Util.h"

// IMU includes
#include <Wire.h>
#ifndef DISABLE_IMU
	#include "Wire.h"
	#include "I2Cdev.h"
	#include "dmpMPU9150.h"
	#include "sensors/MPUSensor.h"
#endif

#ifndef DISABLE_ZYGOTE
	#include <LinkedList.h>
#endif

// Sensors
#include <AnalogIn.h>
#include <DigitalIn.h>
#include <sensors/SharpLong.h>
#include <sensors/SharpLong.h>
#include <sensors/EZUltrasonicAnalog.h>
#include <sensors/HMC6352.h>
#include "LightSensor.h"
#include <BallRadar.h>

// Operational System
#include <Thread.h>
#include <ArduinOS.h>
#include <views/Button.h>
#include <views/CheckBox.h>
#include <views/SeekBar.h>
#include <views/TextView.h>
#include <views/AngleView.h>
#include <ArduinoSensors.h>

//Motors
#include "MoveDriver.h"
#include "MoveController.h"
#include "PololuDriver.h"

// Basic robot definitions
#include "Config.h"
#include "Robot.h"

//Utility classes
#include <Gaussian.h>
#include <GaussianAverage.h>

#include "CompassPID.h"

// Activity includes
#include "MainActivity.h"
#include "TestMotorsActivity.h"
#include "TestUltrasonicActivity.h"
#include "TestBallSensorsActivity.h"
#include "TestCompassActivity.h"
#include "CalibrateActivity.h"
#include "TestLineDetectionActivity.h"
#include "TestBrakesActivity.h";
#include "ChaseBallActivity.h"
#include "ForwardActivity.h"
#include "GoalkeeperActivity.h"

#ifndef DISABLE_IMU
	#include "TestIMUActivity.h"
#endif

#ifndef DISABLE_ZYGOTE
	MainActivity zygote = MainActivity();
#endif

Button back = Button();

char c;
void serial(){

	/*if(PC.available() > 0){
		c = PC.read();

		if(!c)
		return;

        // Serial.print("$ thread: got: ");
        // Serial.println(c);
        Activity* act = ActivityManager::getCurrentActivity();
        if(c == 'p'){
            // Serial.print("$ Thread: pausing Activity: ");
            // Serial.println(act->ActivityID);
            act->pause();
            }else if(c == 'r'){
            // Serial.print("$ Thread: resuming Activity: ");
            // Serial.println(act->ActivityID);
            act->resume();
            }else if(c == 's'){
            // Serial.print("$ Thread: resuming Activity: ");
            // Serial.println(act->ActivityID);
            act->stop();
            }else if(c == 'k'){
            // Serial.print("$ Thread: resuming Activity: ");
            // Serial.println(act->ActivityID);
            act->kill();
            }else if(c == 'g'){
            	PC.print("$ Thread: Current Activity: ");
            	PC.println(act->ActivityID);
            // act->kill();
        }
    }*/

    // if(PC.available()){
    // 	Serial.println();

	   //  while(PC.available() > 0)
	   //  	PC.print(PC.read());
	    	
    // 	Serial.println();
    // }

    // if(BT.available() > 0){
    // 	PC.write(BT.read());
    // }
}

void goBack(){
	beep();
	ActivityManager::getCurrentActivity()->stop();
}

void setup(){
	initRobot();

	// Timer.getAvailable().attachInterrupt(serial).start(50);
	
	ChaseBallActivity *chaseBallAct = new ChaseBallActivity(MC, Ball);
	TestMotorsActivity *testMotorsAct = new TestMotorsActivity(MC);
	TestUltrasonicActivity *testUltrasonicAct = new TestUltrasonicActivity();
	TestBallSensorsActivity *testBallSensorsAct = new TestBallSensorsActivity();
	TestCompassActivity *testCompassAct = new TestCompassActivity();
	TestLineDetectionActivity *testLineAct = new TestLineDetectionActivity(lineEnabled, MC);
	TestBrakesActivity *testBrakesAct = new TestBrakesActivity(MC);
	ForwardActivity *forwardAct = new ForwardActivity(MC);
	GoalkeeperActivity *goalkeeperAct = new GoalkeeperActivity(MC);
	#ifndef DISABLE_IMU
		TestIMUActivity *testIMUAct = new TestIMUActivity();
	#endif

	#ifndef DISABLE_ZYGOTE

		zygote.activities->add(chaseBallAct);
		zygote.activities->add(testMotorsAct);
		zygote.activities->add(testUltrasonicAct);
		zygote.activities->add(testBallSensorsAct);
		zygote.activities->add(testCompassAct);
		zygote.activities->add(testLineAct);
		zygote.activities->add(testBrakesAct);
		zygote.activities->add(forwardAct);
		zygote.activities->add(goalkeeperAct);

		#ifndef DISABLE_IMU
			zygote.activities->add(testIMUAct);
		#endif
	
		zygote.o.x = 5;
		zygote.o.y = 5;
		zygote.initialize();
	#endif

	back.setArea(640,400,150,40);
	back.name = String("Return");
	back.onClick(goBack);

	chaseBallAct->addView(&back);
	testMotorsAct->addView(&back);
	testUltrasonicAct->addView(&back);
	testBallSensorsAct->addView(&back);
	testBrakesAct->addView(&back);
	forwardAct->addView(&back);
	goalkeeperAct->addView(&back);
	//testCompassAct->addView(&back);
	
	#ifndef DISABLE_IMU
		testIMUAct->addView(&back);
	#endif
}

void loop(){
	PC.println("====== START OF PROGRAM ======");
    // Start Activity
    #ifndef DISABLE_ZYGOTE
    	zygote.start();
    #else
    	// Enter code here to run "custom" zygote
	#endif
    // End of Program
    PC.println("====== END OF PROGRAM ======");
}