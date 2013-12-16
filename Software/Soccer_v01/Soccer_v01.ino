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
	#include "sensors/IMUSensor.h"
#endif

#ifndef DISABLE_ZYGOTE
	#include <LinkedList.h>
#endif

// Analog Sensors
#include <AnalogIn.h>
#include <sensors/SharpLong.h>
//#include <sensors/SharpLongMod.h>
#include <sensors/EZUltrasonicAnalog.h>

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
#include "SoccerLocator.h"

// Activity includes
#include "MainActivity.h"
#include "TestIMUActivity.h"
#include "TestMotorsActivity.h"
#include "TestSharpActivity.h"
#include "TestUltrasonicActivity.h"
#include "TestBallSensorsActivity.h"
#include "TestSoccerLocatorActivity.h"
#include "GoalkeeperActivity.h"
#include "ForwardActivity.h"
#include "CalibrateActivity.h"
#include "FuckingCrazyActivity.h"

#ifndef DISABLE_ZYGOTE
	MainActivity zygote = MainActivity();
#endif


Activity act1 = Activity();
Activity act2 = Activity();
Activity act3 = Activity();

Button v = Button();
Button back = Button();
SeekBar pb, pbup;

char c;
void serial(){

	if(PC.available() > 0){
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
    }
}

void runAct2(){
	beep();
	act2.start(false);
}

void goBack(){
	beep();
	ActivityManager::getCurrentActivity()->stop();
}


void onUpdateProgress(){
	// pb.setValue((pb.getValue()+1 > pb.getMax()? 0: pb.getValue() + 1));
	// pbup.setValue((pbup.getValue()+1 > pbup.getMax()? 0: pbup.getValue() + 1));
	delayMicroseconds(pb.getValue()*10);
}

void checkbox_changed(bool val){
	digitalWrite(pTEMP_COOLER, val);
	beep();
	// Serial.print("CALLBACK: val=");
	// Serial.println(val);
}

void setup(){
	initRobot();

	Timer.getAvailable().attachInterrupt(serial).start(50);

	act1.ActivityName = "ListView test";
	act2.ActivityName = "Self test robot";
	act3.ActivityName = "Views";
	
	TestMotorsActivity *testMotorsAct = new TestMotorsActivity(MC);
	TestSharpActivity *testSharpAct = new TestSharpActivity();
	TestUltrasonicActivity *testUltrasonicAct = new TestUltrasonicActivity();
	TestBallSensorsActivity *testBallSensorsAct = new TestBallSensorsActivity();
	TestSoccerLocatorActivity *testLocatorAct = new TestSoccerLocatorActivity();
	GoalkeeperActivity *goalkeeperAct = new GoalkeeperActivity(MC);
	ForwardActivity *forwardAct = new ForwardActivity(MC);
	FuckingCrazyActivity *crazyAct = new FuckingCrazyActivity(MC);
#ifndef DISABLE_IMU
	TestIMUActivity *testIMUAct = new TestIMUActivity();
#endif
	#ifndef DISABLE_ZYGOTE
		//zygote.activities->add(&act1);
		//zygote.activities->add(&act2);
		//zygote.activities->add(&act3);
		zygote.activities->add(testMotorsAct);
		zygote.activities->add(testSharpAct);
		zygote.activities->add(testUltrasonicAct);
		zygote.activities->add(testBallSensorsAct);
		zygote.activities->add(testLocatorAct);
		zygote.activities->add(new CalibrateActivity(MC));
		zygote.activities->add(goalkeeperAct);
		zygote.activities->add(forwardAct);
		zygote.activities->add(crazyAct);
		#ifndef DISABLE_IMU
			zygote.activities->add(testIMUAct);
		#endif
		zygote.o.x = 5;
		zygote.o.y = 5;
		zygote.initialize();
	#endif

	back.setArea(600,350,200,80);
	back.name = String("Return");
	back.onClick(goBack);
	act1.addView(&back);
	act2.addView(&back);
	act3.addView(&back);
	testMotorsAct->addView(&back);
	testSharpAct->addView(&back);
	testUltrasonicAct->addView(&back);
	testBallSensorsAct->addView(&back);
	testLocatorAct->addView(&back);
	goalkeeperAct->addView(&back);
	forwardAct->addView(&back);
#ifndef DISABLE_IMU
	testIMUAct->addView(&back);
#endif
	CheckBox *check = new CheckBox("Enable?");
	// check->onChange(checkbox_changed);
	check->o = Point(30,10);
	act3.addView(check);

	CheckBox *check2 = new CheckBox("");
	check2->onChange(checkbox_changed);
	check2->o = Point(30,100);
	check2->w = 100;
	check2->h = 100;
	act3.addView(check2);




	// Progress bar testing
	Thread* updatPbBar = new Thread();
	updatPbBar->onRun(onUpdateProgress);
	updatPbBar->setInterval(0);
	act3.Threads.add(updatPbBar);

	pb = SeekBar();
	pb.o.x = 200;
	pb.o.y = 10;
	pb.w = 500;
	pb.h = 30;
	pb.setMax(100);
	act3.addView(&pb);

	pbup = SeekBar();
	pbup.o.x = 200;
	pbup.o.y = 250;
	pbup.w = 30;
	pbup.h = 200;
	pbup.type = ProgressBar::LINEAR_UP;
	// pbup.setMax(1.0);
	act3.addView(&pbup);

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