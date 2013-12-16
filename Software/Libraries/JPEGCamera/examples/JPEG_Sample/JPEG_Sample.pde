/*
JPEG Camera Example Sketch
The sketch will take a picture on the JPEG Serial Camera and store the jpeg to an SD card
on an SD Shield
Written by Ryan Owens
SparkFun Electronics

Hardware Notes:
This sketch assumes the arduino has the microSD shield from SparkFun attached.
The camera Rx/Tx should be attached to pins 2 and 3.
IMPORTANT: The JPEG camera requires a TTL level shifter between the camera output
and the arduino. Bypassing this may damage the Arduino pins.
*/

//This example requires the MemoryCard, SdFat, JPEGCamera and NewSoftSerial libraries
// #include <MemoryCard.h>
// #include <SdFat.h>
#include <JPEGCamera.h>

//Create an instance of the camera
JPEGCamera camera;

//Create a character array to store the cameras response to commands
char response[32];
//Count is used to store the number of characters in the response string.
unsigned int count=0;
//Size will be set to the size of the jpeg image.
int size=0;
//This will keep track of the data address being read from the camera
int address=0;
//eof is a flag for the sketch to determine when the end of a file is detected
//while reading the file data from the camera.
int eof=0;

void setup()
{
    //Setup the camera, serial port and memory card
    camera.begin();
    Serial.begin(115200);
    while(!Serial);
    Serial.println("START!");
    // MemoryCard.begin();
    
    //Reset the camera
    count=camera.reset(response);
    delay(3000);
    
    //Take a picture
    count=camera.takePicture(response);
    Serial.print("picture");
    //Print the response to the 'TAKE_PICTURE' command.
    Serial.write((const uint8_t*)response, count);
    Serial.println();
    Serial.print("write");
    
    //Get the size of the picture
    count = camera.getSize(response, &size);
    //Print the size
    Serial.print("Size: ");
    Serial.println(size);
    
    //Create a file called 'test.txt' on the SD card.
    //NOTE: The memoryCard libary can only create text files.
    //The file has to be renamed to .jpg when copied to a computer.
    // MemoryCard.open("/test.txt", true);
    //Starting at address 0, keep reading data until we've read 'size' data.
    while(address < size)
    {
        //Read the data starting at the current address.
        count=camera.readData(response, address);
        //Store all of the data that we read to the SD card
        for(int i=0; i<count; i++){
            //Check the response for the eof indicator (0xFF, 0xD9). If we find it, set the eof flag
            if((response[i] == (char)0xD9) && (response[i-1]==(char)0xFF))eof=1;
            //Save the data to the SD card
            // MemoryCard.file.print(response[i], BYTE);
            Serial.print(response[i],HEX);
            //If we found the eof character, get out of this loop and stop reading data
            if(eof==1)break;
        }
        //Increment the current address by the number of bytes we read
        address+=count;
        //Make sure we stop reading data if the eof flag is set.
        if(eof==1)break;
    }
    //Close the file
     // MemoryCard.close();
     Serial.print("Done.");
}

void loop()
{

}
