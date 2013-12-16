#include <inttypes.h>
#include <Arduino.h>

#include "math.h"
#include "ArduinoSensors.h"
#include "LinkedList.h"
#include "Gaussian.h"
#include "GaussianAverage.h"

#define BUSY 	8
#define COMMON 	7
#define ENABLE	6

#define SMASK   0b00111100
#define S0 		2
#define S1 		3
#define S2 		4
#define S3 		5

#define N_INTENSITIES 8

int lastIntensities[N_INTENSITIES];
int currIntensity;
long lastMillis;

long sum[16];

uint8_t AnalogIR_Port[8] = {
	A1,
	A0,
	A6,
	A7,
	A4,
	A2,
	A3,
	A5,
};

uint8_t DigitalIR_Port[16] = {
	0b00000100, // IR 1
	0b00000101, // IR 2
	0b00000110, // IR 3
	0b00000111, // IR 4
	0b00001000, // IR 5
	0b00001001, // IR 6
	0b00001010, // IR 7
	0b00001011, // IR 8
	0b00001100, // IR 9
	0b00001101, // IR 10
	0b00001110, // IR 11
	0b00001111, // IR 12
	0b00000000, // IR 13
	0b00000001, // IR 14
	0b00000010, // IR 15
	0b00000011, // IR 16
};

/*
	Output registers to S0-S3,
	outputing the right sequence
	to connect the "ir" to common

	ir: 0 - 15
*/
void selectDigitalChannel(uint8_t ir){

	PORTD = (PORTD & 0b11000011) | (DigitalIR_Port[ir] << S0);

	// Let changes apply to the COMMON port
	asm("nop\n\t");
	asm("nop\n\t");
}
/*
	Read selected digital channel
*/
bool readDigitalChannel(){
	return PIND & (0x01 << COMMON);
}

void setup(){
	Serial.begin(57600);
	while(!Serial);
	// Serial.println("$: Initialized");

	pinMode(BUSY, OUTPUT);

	pinMode(ENABLE, OUTPUT);
	digitalWrite(ENABLE, LOW);	// Enable HC4067

	pinMode(S0, OUTPUT);
	digitalWrite(S0, LOW);
	pinMode(S1, OUTPUT);
	digitalWrite(S1, LOW);
	pinMode(S2, OUTPUT);
	digitalWrite(S2, LOW);
	pinMode(S3, OUTPUT);
	digitalWrite(S3, LOW);

	pinMode(COMMON, INPUT);
	digitalRead(COMMON);
	
	currIntensity = 0;
}


int loops = 0;
long distance = 0;
long avg = 0;

long IR_DigitalValue[16];
long IR_IntegralValue = 0;

#define IR_LOOPS 		550
#define IR_MIN_INTEGRAL 100
#define IR_MIN_REPEATS	10

void getSums() {
	// Clear actual values
	for(int i = 0; i < 16; i++){
		sum[i] = 0;
	}

	// Busy LED on
	digitalWrite(BUSY, HIGH);

	// For each sensor on the MUX...
	for(int i = 0; i < 16; i++){
		selectDigitalChannel(i);

		// Read IR_LOOPS times
		for(long x = 0; x < IR_LOOPS; x++){
			sum[i] += !readDigitalChannel();
			if(sum[i] >= IR_MIN_INTEGRAL)
				break;
		}

	}
	IR_IntegralValue = 0;

	// Filter values, and send to IR_DigitalValue
	for(int i = 0; i < 16; i++){

		// Don't let go farter than IR_MIN_REPEATS, and below 0
		IR_DigitalValue[i] =
			min(IR_MIN_REPEATS,
				max(IR_DigitalValue[i] + (sum[i] >= IR_MIN_INTEGRAL? 1:-1), 0));

		IR_IntegralValue += IR_DigitalValue[i];
	}
	digitalWrite(BUSY, LOW);
}

long xInt = 0;
long yInt = 0;
int getIntensity() {
	int maxIntensity = 0;
	xInt = 0;
	yInt = 0;
	for(int i = 0; i < 8; i++){
		long tempInt = analogRead(AnalogIR_Port[i]);
		maxIntensity = max(maxIntensity, tempInt);

		xInt += cos(i * M_PI/4.0) * tempInt;
		yInt += sin(i * M_PI/4.0) * tempInt;
	}
	return maxIntensity;
}

int getMaxIntensity() {
	int ans = 0;
	for (int i = 0; i < N_INTENSITIES; i++)
		ans = max(ans, lastIntensities[i]);
	return ans;
}

//Returns the index, if any, of the first sensor of a continuous group of size qtt which detects the ball
/*int getDirectionWith(int qtt) {
	for (int i = 0; i < 16; i++) {
		//first and last
		if (!IR_DigitalValue[i] || !IR_DigitalValue[(i + qtt - 1) % 16])
			continue;
		
		int qtt_off = 0;
		for (int j = i; j < i + qtt; j++) {
			if (!IR_DigitalValue[j % 16])
				qtt_off++;
		}
		
		if (qtt_off > 30*qtt/100) //at most x % not detecting
			continue;
			
		//return the middle, multiplying by 2 and creating virtual sensors in each gap
		if (qtt & 1)
			return (2 * (i + (qtt / 2))) % 32;
		else
			return (1 + 2 * (i + (qtt / 2 - 1))) % 32;
	}
	
	return 80;
}*/

#define MAX_SENSORS 16
#define K_ANGLE	0

float _tempAngles[MAX_SENSORS] = {
    K_ANGLE + 0.3927 * 0.5,
    K_ANGLE + 0.3927 * 1.5,
    K_ANGLE + 0.3927 * 2.5,
    K_ANGLE + 0.3927 * 3.5,
    K_ANGLE + 0.3927 * 4.5,
    K_ANGLE + 0.3927 * 5.5,
    K_ANGLE + 0.3927 * 6.5,
    K_ANGLE + 0.3927 * 7.5,
    K_ANGLE + 0.3927 * 8.5,
    K_ANGLE + 0.3927 * 9.5,
    K_ANGLE + 0.3927 * 10.5,
    K_ANGLE + 0.3927 * 11.5,
    K_ANGLE + 0.3927 * 12.5,
    K_ANGLE + 0.3927 * 13.5,
    K_ANGLE + 0.3927 * 14.5,
    K_ANGLE + 0.3927 * 15.5
};

float _tempWeights[MAX_SENSORS] = {
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0,
    10.0
};

// Filters
GaussianAverage kalmanIntensity(3);
GaussianAverage kalmanDirectionX(3);
GaussianAverage kalmanDirectionY(3);

void printSums() {
	for (int i = 0; i < 16; i++) {
		Serial.print(IR_DigitalValue[i]);
		Serial.print("\t");
	}
	Serial.println();
}

int noBall = 0;

// #define DEBUG	1
int debugDigital = 0;

void loop(){
	
	// DEBUG
	#ifdef DEBUG
		if(Serial.available()){
			while(Serial.available()) Serial.read();
			debugDigital = (debugDigital+1) % 16;
			Serial.print("\t\tActual: ");
			Serial.println(debugDigital);
		}
	#endif

	// Reads intensity, and saves into a circular buffer
	lastIntensities[currIntensity++] = getIntensity();
	if (currIntensity == N_INTENSITIES)
		currIntensity = 0;

	// Read digital Sensors
	getSums();

	if (millis() - lastMillis >= 50) {

		// Get max intensity from sensors
		int maxRawIntensity = getMaxIntensity();

		// Creates a gaussian object to store the intensity and it's variance
		Gaussian gIntensity = Gaussian(maxRawIntensity);
		gIntensity.variance = 10 + abs(kalmanIntensity.process().variance - maxRawIntensity);
		// gIntensity.variance = gIntensity.mean;

		// Add current value to the Gaussian Filter
		kalmanIntensity.add(gIntensity);

		// DEPRECATED
		// int direction = getDirection();

		// Calculate RAW vector modules for measurements
	    double rawX = 0.0;
	    double rawY = 0.0;

	    // #ifndef DEBUG
		    for(int i = 0; i < MAX_SENSORS; i++){
		    	// Serial.print(IR_DigitalValue[i]);
		    	// Serial.print("\t");

		        rawX += (IR_DigitalValue[i]*_tempWeights[i])*sin(_tempAngles[i]);
		        rawY += (IR_DigitalValue[i]*_tempWeights[i])*cos(_tempAngles[i]);
		    }
	    /*#else
	    	rawX += (IR_DigitalValue[debugDigital]*_tempWeights[debugDigital])*sin(_tempAngles[debugDigital]);
	        rawY += (IR_DigitalValue[debugDigitaül]*_tempWeights[debugDigital])*cos(_tempAngles[debugDigital]);
	    #endif*/

	    // Serial.print("\n");
	    double directionDouble;
	    bool ballNotDetected = false;

	    if(rawY == 0.0 && rawX == 0.0){
	    	// noBall ++;

	    	// Check if ball has not been detected for a long period of time
			if(noBall > 8){
				// Set the flag to true
				ballNotDetected = true;
			}

	    }else{
	    	noBall = 0;

	    	// Actual direction, in Radians
		    directionDouble = (atan2(rawX, rawY));
		    
		    double normalizedX = cos(directionDouble);
		    double normalizedY = sin(directionDouble);

		    kalmanDirectionX.add(
		    	Gaussian(
		    		normalizedX));//,
		    		//1.0 + abs(100 * (normalizedX - kalmanDirectionX.process().variance)) ));

			kalmanDirectionY.add(
				Gaussian(
					normalizedY));//,
					//1.0 + abs(100 * (normalizedY - kalmanDirectionY.process().variance)) ));

			// Direction is the current RAW direction, read from sensors
		    // direction = constrain( ((int)( toDegs(directionDouble + M_PI) / 11.25)+16) % 32, 1, 127);
		}
		// Filter vectors and saves current x and y
		double filteredX = kalmanDirectionX.process().mean;
		double filteredY = kalmanDirectionY.process().mean;

		// Filtered Angle data, using pseudo-kalman filter
		// + M_PI is to convert from -180/180 to 0/360
		double finalAngle = toDegs(atan2(filteredY, filteredX) + M_PI);

		// This is the byte that is going to be sent to the 
		uint8_t chunkedAngle = 0xFF;
		if(!ballNotDetected)
			chunkedAngle = (uint8_t)(constrain(finalAngle, 0, 360) / 3);

		// Serial.print(angleFinal);
		// Serial.print("\t");
		// Serial.println(direction);

		int intensity = constrain(map(kalmanIntensity.process().mean, 20, 400, 0, 255), 0, 255);
		
		/*
			Protocol definition:
				* 2 Bytes
				* Bit 0 defines TYPE
				
				Case Bit 0 = 0:
					Message is Direction in degrees, divided by 3;

				Case Bit 0 = 1:
					Message is Intensidty;
		*/
		//Bit to indicate type: 1 -> intensity(force odd), 0 -> direction (left shifted to preserve value)
		uint8_t dirByte = chunkedAngle << 1;
		uint8_t intByte = (ballNotDetected? 0 : intensity) |  1;
		


		//Write as simple messages; secured by least significant bit
		#ifdef DEBUG
			// if(angleFinal != 80)
			Serial.print(ballNotDetected*1);
			Serial.print(" | ");
			Serial.print( chunkedAngle );
			Serial.print("\t| ");
			Serial.print( intensity );
			// Serial.print("\t| ");
			Serial.print("\t| X:");
			Serial.print( rawX );
			Serial.print("\t| Y:");
			Serial.println( rawY );
			// printSums();
		#else
			Serial.write(dirByte);
			Serial.write(intByte);
		#endif

		lastMillis = millis();
	}
}
