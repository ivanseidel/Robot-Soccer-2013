void setup() {
  // put your setup code here, to run once:
  Serial3.begin(57600);
  Serial.begin(115200);
  
  delay(100);
  
  delay(2000);
  Serial.println("Starting...");
  delay(1000);
}

void loop() {
  	// put your main code here, to run repeatedly: 
	if(Serial3.available()){
	  	Serial.print("Got:\t");
	    Serial.println(Serial3.read());
	}
}
