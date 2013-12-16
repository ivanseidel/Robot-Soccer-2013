#include <inttypes.h>
#include <Arduino.h>

#define BUSY 	8
#define COMMON 	7
#define ENABLE	6

#define SMASK   0b00111100
#define S0 		2
#define S1 		3
#define S2 		4
#define S3 		5

#define N_INTENSITIES 30

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
//	Serial.println("$: Initialized");

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

#define IR_LOOPS 		550
#define IR_MIN_INTEGRAL 100
#define IR_MIN_REPEATS	2

void getSums() {
	for(int i = 0; i < 16; i++){
		sum[i] = 0;
	}

	digitalWrite(BUSY, HIGH);
	for(int i = 0; i < 16; i++){
		selectDigitalChannel(i);

		for(long x = 0; x < IR_LOOPS; x++){
			sum[i] += !readDigitalChannel();
			if(sum[i] >= IR_MIN_INTEGRAL)
				break;
		}

	}

	// Filter values, and send to IR_DigitalValue
	for(int i = 0; i < 16; i++){

		// Don't let go farter than IR_MIN_REPEATS, and below 0
		IR_DigitalValue[i] =
			min(IR_MIN_REPEATS,
				max(IR_DigitalValue[i] + (sum[i] >= IR_MIN_INTEGRAL? 1:-1), 0));
	}
	digitalWrite(BUSY, LOW);
}

int getIntensity() {
	int maxIntensity = 0;
	for(int i = 0; i < 8; i++){
		maxIntensity = max(maxIntensity, analogRead(AnalogIR_Port[i]));
	}
	return maxIntensity;
}

int getMaxIntensity() {
	int ans = 0;
	for (int i = 0; i < N_INTENSITIES; i++)
		ans = max(ans, lastIntensities[i]);
	return ans;
}

int getDirectionWith(int qtt) {
	for (int i = 0; i < 16; i++) {
		bool ok = true;
		for (int j = i; j < i + qtt; j++) {
			if (!IR_DigitalValue[j % 16]) {
				ok = false;
				break;
			}
		}
		if (ok) {
			if (qtt & 1)
				return 2 * (i + (qtt / 2));
			else
				return 1 + 2 * (i + (qtt / 2 - 1));
		}
	}
	
	return 255;
}

int getDirection() {
	//SENSOR COM PROBLEMA
	IR_DigitalValue[2] = 0;
	if (IR_DigitalValue[1] && IR_DigitalValue[3])
		IR_DigitalValue[2] = 2;
	
	for (int qtt = 8; qtt >= 1; qtt--) {
		int dir = getDirectionWith(qtt);
		if (dir != 255)
			return dir;
	}
	
	return 255;
}

void printSums() {
	for (int i = 0; i < 16; i++) {
		Serial.print(IR_DigitalValue[i]);
		Serial.print("\t");
	}
	Serial.println();
}

void loop(){
	getSums();
	
	lastIntensities[currIntensity++] = getIntensity();
	if (currIntensity == N_INTENSITIES)
		currIntensity = 0;

	if (millis() - lastMillis >= 200) {
		int direction = getDirection();
		int intensity = constrain(map(getMaxIntensity(), 20, 400, 0, 255), 0, 255);
		
		byte data[2] = { direction, intensity };
		Serial.print(data[0]);
		Serial.print("\t");
		Serial.println(data[1]);
		
		lastMillis = millis();
	}
}
