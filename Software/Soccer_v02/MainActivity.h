/*
	If you need to create a simple menu,
	to run many activities, than this
*/
#ifndef DISABLE_ZYGOTE

#define MENU_WIDTH	790
#define MENU_HEIGHT	50
#define MENU_SPACE	2

#include <LinkedList.h>
#include <views/Button.h>

class MainActivity: public Activity, public Thread{
protected:

	class CustomButton: public Button{
	public:
		Activity* actToStart;
		
		CustomButton(){
			actToStart = 0;

			Button::Button();

			cText = color(0xFF, 0xFF, 0xFF);
		};

		void onClick(){
			beep();
			if(actToStart)
				actToStart->start(false);
		};
	};

public:
	LinkedList<Activity*>* activities;

	void initialize(){
		for(int i = 0; i < activities->size(); i++){
			Activity* _act = activities->get(i);
			CustomButton* btn = new CustomButton();

			btn->name = _act->ActivityName;
			btn->setArea(
				24,
				MENU_HEIGHT*i,
				MENU_SPACE + MENU_WIDTH - 28,
				MENU_HEIGHT - MENU_SPACE);
			// Register Activity to start callback
			btn->actToStart = _act;

			// Detail View
			Button* v = new Button();
			v->_requestTouch = false;
			v->setArea(
				0,
				MENU_HEIGHT*i,
				22,
				MENU_HEIGHT - MENU_SPACE);
			v->cBg = color(240,200,31);
			v->name = String("");

			addView(btn);
			addView(v);
		}
	};

	MainActivity(){
		ActivityName = String("Soccer v2.0 - Main Menu");
		_enableReturn = false;

		// Initialize LinkedList
		activities = new LinkedList<Activity*>();

		Activity::Activity();

		setInterval(100);
		Threads.add(this);
	};

	virtual void run(){
		// Serial.println("Sistem OK...");
		Serial.println(digitalRead(pBTN_PAUSE));
		// Serial.print("Compass: ");
		// long start = micros();
		// float val = Compass->readAngle();
		// long end = micros();
		// Serial.print(val);
		// Serial.print("\t");
		// Serial.println(end-start);

		runned();
	}

	void onCreate(){
		beep();
		delay(80);
		beep();

		initialize();
	}

	void onLoop(){


		// delay(500);
		//delay(1000);
		//Serial.println("alive");
		// Serial.println(digitalRead(pBTN_PAUSE));
	}



};

#endif