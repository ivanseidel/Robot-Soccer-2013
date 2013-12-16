/* Arduino JPeg Camera Library
 * Copyright 2010 SparkFun Electronics
 * Written by Ryan Owens
*/

#ifndef JPEGCamera_h
#define JPEGCamera_h

#if defined(__AVR__)
    #include <avr/pgmspace.h>
    #define fontdatatype uint8_t
#elif defined(__PIC32MX__)
    #define PROGMEM
#elif defined(__arm__)
    #define PROGMEM
#endif


#include "Arduino.h"

class JPEGCamera
{
	public:
		JPEGCamera();
		void begin(void);
		int reset(char * response);
		int getSize(char * response, int * size);
		int takePicture(char * response);
		int stopPictures(char * response);
		int readData(char * response, int address);
		
	private:
		int sendCommand(const char * command, char * response, int length);
};

#endif