#include <Arduino.h>

// Motors incluedes
#include "PololuDriver.h"
#include "MoveController.h"
// #include "mbed_driver.h"

// General peripheral includes
// #include <DueTimer.h>
// #include <UTouch.h>
// #include <UTFT.h>

// Operational System
// #include <ArduinOS.h>

// Include Icons and fonts
// #include <fGUI.c>

/*
	GLOBAL Sensors objects
*/
// IMU setup
#ifndef DISABLE_IMU
	IMUSensor *IMU;
#endif

// SENSORS setup
#ifndef DISABLE_SENSORS
	
#endif

/*
    MoveController definitions
*/
#ifndef DISABLE_MOTION
	PololuDriver *MD;
	MoveController *MC;
#endif

#define FRONT 0
#define LEFT 1
#define BACK 2
#define RIGHT 3

/*
	LED Status timer interrupt
*/
/*void irqLedStatus(){
	static bool active = false;

	 PIO_SetOutput( g_APinDescription[pLED_STATUS].pPort, g_APinDescription[pLED_STATUS].ulPin, active, 0, PIO_PULLUP ) ;
}*/

/*
	RETURN Button implementation
*/
void irqReturnBtn(){
	// Serial.println("PRESS!");
	static long lastCall = 0;
	// Check if there was a minimum time between clicks (debounce)
	if(millis()- lastCall < osRETURN_MIN_TIME)
		return;

	lastCall = millis();
	ArduinOS::actionHandler(ACTION_RETURN);
}

void irqPauseBtn(){
	//static long lastCall = 0;
	
	bool pause = digitalRead(pBTN_PAUSE);
	
	Serial.println(pause ? "Pause!" : "Not paused =(");
	
	Activity* act = ActivityManager::getCurrentActivity();
    if(pause)
        act->pause();
    else
        act->resume();
	delay(1);
}

/*
	Beep function and Thread
*/
// Used to warn battery
Thread thrBeeper;
void onBeeper(){
	static bool beeping = false;
	beeping = !beeping;
	digitalWrite(pBUZER, beeping);
	if(beeping)
		thrBeeper.setInterval(TIME_BEEP_H);
	else
		thrBeeper.setInterval(TIME_BEEP_L);
}

// Notify touchs
void beep(){
	digitalWrite(pBUZER, HIGH);
	delayMicroseconds(10000);
	digitalWrite(pBUZER, LOW);
}


#ifndef DISABLE_LCD
	/*
		LCD and Touch definitions
	*/
	UTFT LCD(ITDB50,pLCD_RS,pLCD_WR,pLCD_CS,pLCD_RESET);
	UTouch TCH = UTouch(pTCH_CLK,pTCH_CS,pTCH_DIN,pTCH_OUT,pTCH_IRQ);

	// extern uint8_t SmallFont[];			// Font
	// extern uint8_t BigFont[];		// Font
	extern uint8_t OCR_A_Extended_M[];	// Font
	// extern uint8_t SevenSegNumFont[];	// Font
	extern uint8_t fGUI[];				// Font


	/*
		System GUI (General User Interface)
	*/
	Thread thrUpdateGUI;
	View GUI = View();
	void onGUIupdate(){
		static float voltage, analog;
		static int batteryStatus, failsCount = 0;
		LCD.setBackColor(cGUITopBarFill);
		
		// Print BT status
		LCD.setFont(fGUI);
		LCD.setColor(180,180,180);
		LCD.printNumI(6, 1,1);
		

		// Calculates battery voltage and current status
		analog = analogRead(pVIN);
		voltage = max((analog-175)/86.25 + 5.3, 0);

		if(analog <= VOLTAGE_USB){
			batteryStatus = fGUI_USB;	// On USB
		}else{
			batteryStatus = 
				max(0,min(5,map(voltage*100, VOLTAGE_LOW*100, VOLTAGE_HIGH*100,
					0,5)));
		}

		if(batteryStatus == 0){
			if(++failsCount >= 3)
				thrBeeper.enabled = true;	// Enable Beeper thread
			LCD.setColor(240,20,30);	// Very low
		}else if(batteryStatus == fGUI_USB){
			LCD.setColor(70,70,70);		// USB Plugged
		}else if(batteryStatus >=  4){
			LCD.setColor(20,200,20);	// Full charge
		}else{
			LCD.setColor(245,210,95);	// Normal
		}

		if(batteryStatus != 0){
			thrBeeper.enabled = false;	// Disable Beeper thread
			digitalWrite(pBUZER, LOW); 	// Stop beeping
			failsCount = 0;
		}

		// Render Battery status
		LCD.printNumI(batteryStatus, 731,1);
		LCD.setFont(fTextBig);
		LCD.setColor(150,150,150);
		if(batteryStatus != fGUI_USB)
			LCD.printNumF(voltage,1, 665,4,'.',4);
		else
			LCD.print("USB",680,4);

		LCD.print(String(ArduinOS::cpuThreadUsage) + "%  ", 78,4);
	}
	
	void onGUIrender(){
		static Activity* act;
		act = ActivityManager::getCurrentActivity();

		LCD.setColor(cGUITopBarFill);
		LCD.fillRect(0,0,LCDWidth, 31);
		LCD.setBackColor(cGUITopBarFill);

		// Render Activity Name
		if(act){
			LCD.setColor(VGA_BLACK);
			LCD.setFont(fTextBig);
			LCD.print(act->ActivityName, CENTER, 4);
		}

		onGUIupdate();
		// PC.println("$: GUI: Rendered.");
	}

	View STAGE = View();
	void onSTAGErender(){
		LCD.setColor(VGA_BLACK);
		LCD.fillRect(ArduinOS::_rel.x,ArduinOS::_rel.y,LCDWidth, LCDHeight);
	}

	void onInvalidateView(){
		ArduinOS::requestRender = true;
	}

#endif	// DISABLE_LCD

ProgressBar preLoader = ProgressBar();

// Perform a FULL initialization and configuration
// of all connected hardware.
void initRobot(){
	
	// Initialize All Serial's
	PC.begin(115200);	// Bluetooth and PC are the same in soccer
	// BLUETOOTH.begin(9600);
	
	DRIVER1_USART.begin(115200);
	DRIVER2_USART.begin(115200);
	
	BALL_USART.begin(57600);

	while(!PC || !BLUETOOTH || !DRIVER1_USART || !DRIVER2_USART || !BALL_USART);

	PC.println("$: Serial initialized");

	// Initialize Pin Ports as I/O
	const int output[] = {
		pLED_STATUS,
		pBUZER,
				
		pLCD_LED,
		
		pSD_CS,
		
		pTEMP_COOLER,
		
		pSERVO_1,
		pSERVO_2,

		pDIST_EN_TOP,
		pDIST_EN_BOT,
		pDIST_TRIG_TOP,
	};

	const int input[] = {
		pBTN_PAUSE,
		pBTN_SW1,
		pBTN_SW2,
		
		pIMU_IRQ,

		pVIN,

		pTEMP_SENSOR,

		pDIST_1,
		pDIST_2,
		pDIST_3,
		pDIST_4,
		pDIST_5,
		pDIST_6,
		pDIST_7,
		pDIST_8,
		pDIST_9,
		pDIST_10,		
	};

	
	for(int i = 0; i < sizeof(output)/sizeof(int); i++){
		pinMode(output[i], OUTPUT);
		digitalWrite(output[i], LOW);
	}

	PC.println("$: Output pins initialized");

	for(int i = 0; i < sizeof(input)/sizeof(int); i++){
		pinMode(input[i], INPUT);
	}

	PC.println("$: Input pins initialized");


	// Initialize Status LED Timer
	// TIMER_LED.attachInterrupt(irqLedStatus).setFrequency(10).start();
	// PC.println("$: Status LED Initialized");

	/*
		ArduinOS setup
	*/
	#ifndef DISABLE_LCD
		// Initialize TFT LCD
		LCD.InitLCD();
	  	LCD.setFont(OCR_A_Extended_M);
	  	LCD.clrScr();
	    // Turn LCD BackLight ON
	    digitalWrite(pLCD_LED, HIGH);

		PC.println("$: LCD Initialized");

	  	// Initialize TOUCH Screen
	  	TCH.InitTouch(LANDSCAPE);
	    TCH.setPrecision(PREC_MEDIUM);
		PC.println("$: TOUCH Initialized");
	#endif
	
	// Preloader
	preLoader.setMax(12);
	preLoader.o = Point(150,230);
	preLoader.w = 500;
	preLoader.setValue(1, true);

	// Thread initialization
	ArduinOS::_threadTimer = &TIMER_THREAD1;
	ArduinOS::SystemThreads = new ThreadController();
	ArduinOS::SystemThreads->ThreadName = "SystemThread";
	ArduinOS::_threadInterval = 10000;
	preLoader.setValue(2, true);

	#ifndef DISABLE_LCD
	    // Configure ArduinOS to read Touch interrupts from LCD
	    ArdUI::touchInterrupt = pTCH_IRQ;
	    ArdUI::touchTimer =  &TIMER_TCH;
	    ArdUI::touchObject = &TCH;
	    ArdUI::touchMode = ArdUI::INTERRUPT_TIMER;
	    ArdUI::touchTimerPeriod  = 50000;
	    ArdUI::LCD = &LCD;

	    ArdUI::onInvalidateView = onInvalidateView;

	    // Configure GUI
	    GUI.onRender(onGUIrender);
	    ArduinOS::GUI = &GUI;
	    STAGE.onRender(onSTAGErender);
	    ArduinOS::STAGE = &STAGE;

	    // Create and register GUI updater on SystemThreads
	    thrUpdateGUI = Thread(onGUIupdate, UPDATE_GUI_INTERVAL);
	    thrUpdateGUI.ThreadName = "Update GUI";
	    ArduinOS::SystemThreads->add(&thrUpdateGUI);
		PC.println("$: GUI Updater Thread registered: "+thrUpdateGUI.ThreadName);
	#endif

    // Configure Return button of ArduinOS
    attachInterrupt(pBTN_SW2, irqReturnBtn, RISING);
	preLoader.setValue(3, true);
	
	// Configure pause button
	//attachInterrupt(pBTN_PAUSE, irqPauseBtn, CHANGE);


	// snipet beep() register on ArduinOS
	ArduinOS::beep = beep;
	preLoader.setValue(4, true);
	
	// Beeping initialization
	thrBeeper = Thread(onBeeper, TIME_BEEP_H);	// Instantiate beeper object
    thrBeeper.ThreadName = "Beeper";			// Default ThreadName
	thrBeeper.enabled = false; 				// Defalut is disabled
    ArduinOS::SystemThreads->add(&thrBeeper);	// Register thread on SystemThreads
    PC.println("$: Beeper Thread registered: "+thrBeeper.ThreadName);
	preLoader.setValue(5, true);

    // IMU Initialization
    #ifndef DISABLE_IMU
    	IMU = new IMUSensor(Wire1);
		while (!IMU->initialize()) {
			delay(100);
		}
		Serial.println(IMU->initialize()? "$: IMU INITIALIZATION SUCCESFULL!": "$: !!!!!!!!!!FAILED TO INITIALIZE IMU!!!!!!!!!!!");
		IMU->readYPR = true;
		IMU->setInterval(40);
		// ArduinOS::SystemThreads->add(IMU);	// We must register LOCALY the Thread
		preLoader.setValue(6, true);
	#endif

	// SENSORS setup
	analogReadResolution(10); // Set 10 bits resolution

	#ifndef DISABLE_SENSORS
		

		preLoader.setValue(8, true);

	#endif

	/*
	    MoveController Initialization
	*/
	#ifndef DISABLE_MOTION
		MD = new PololuDriver();
		MC = new MoveController(MD);
		MC->setSpeed(0);
	#endif


    // Finished
	PC.println("$: ArduiOS Initialized");
	preLoader.setValue(9, true);
}