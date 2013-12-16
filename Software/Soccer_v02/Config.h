#include <Arduino.h>
#include <DueTimer.h>

/*
	ROBOT GENERAL CONFIGURATION
*/

// Timers usage definition, to avoid conflicts
#define TIMER_THREAD1	Timer3		// Timer that will trigger threads on ArduinOS
#define TIMER_TCH		Timer4		// Used for LCD hit tests while moving
#define TIMER_LED		Timer5		// Blinking led, to show that "I'm alive!"


// User Interface (LED's, Buzzer...)
#define pLED_STATUS		8
#define pBTN_PAUSE		3
#define pBTN_SW1		7
#define pBTN_SW2		6

// Buzzer configuration
#define pBUZER			22
#define TIME_BEEP		20
#define TIME_BEEP_H		1 			
#define TIME_BEEP_L		200			

// PC (Just for debugging)
#define PC_USART		Serial
#define PC				Serial 		// To simplify

// Bluetooth
#define BLUETOOTH_USART	Serial
#define BLUETOOTH		Serial
#define BT				Serial

// Motor Driver Controller
#define DRIVER1_USART	Serial1
#define DRIVER2_USART	Serial2

// CO-PROCESSOR (ATMEGA328) with Temperature sensors
#define BALL_USART		Serial3

// LCD
// DB0-DB7:  33-40
// DB8-DB15: 51-44
#define pLCD_RS			41
#define pLCD_WR			42
#define pLCD_CS			43
#define pLCD_RESET		53
#define pLCD_LED		9

#define LCDWidth		799
#define	LCDHeight		479

// TOUCH
#define pTCH_CLK		27
#define pTCH_CS			28
#define pTCH_DIN		29
#define pTCH_OUT		30
#define pTCH_IRQ		31

// IMU
#define IMU_WIRE		Wire1
#define pIMU_SDA		SDA1
#define pIMU_SCL		SCL1
#define pIMU_IRQ		32

// SD Card
// Uses SPI pins, no need for definition
#define pSD_CS			52

// Battery voltage
#define pVIN			A11
#define VOLTAGE_USB		130			// In raw units of analogRead()
#define VOLTAGE_LOW		6.4
#define VOLTAGE_HIGH	8.2

// Temperature Sensor and Cooler definitions
#define pTEMP_SENSOR	A10
#define pTEMP_COOLER	11

// Servo Configuration
#define pSERVO_1		12
#define pSERVO_2		13

// Light sensor pin
#define pLIGHT_SENSOR	A0

// IR and US Distance Sensor
#define pDIST_EN_TOP	4
#define pDIST_EN_BOT	5
#define pDIST_TRIG_TOP	10

#define pDIST_1			A0
#define pDIST_2			A1
#define pDIST_3			A2
#define pDIST_4			A3
#define pDIST_5			A4
#define pDIST_6			A5
#define pDIST_7			A6
#define pDIST_8			A7
#define pDIST_9			A8
#define pDIST_10		A9

int pSENSOR[] = {
	pDIST_1,
	pDIST_2,
	pDIST_3,
	pDIST_4,
	pDIST_5,
	pDIST_6,
	pDIST_7,
	pDIST_8,
	pDIST_9,
	pDIST_10
};

/*
	LCD Schemes
*/
#define cRectFill		200,200,200
#define cTextNeutral	30,30,30
#define cGUITopBarFill	255,255,255

#define fTextBig		OCR_A_Extended_M
#define fTextSmall		SmallFont
#define fNumBig			SevenSegNumFont

/*
	ArduinOS Configuration
*/
#define osRETURN_MIN_TIME	100		// Minimum time between clicks on return Btn
									// to debounce sensor. (in Milliseconds)
									
#define UPDATE_GUI_INTERVAL	2000	// Time to update GUI (constantly)
									// Also compute the remaining battery

#define fGUI_USB			9