void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
}

void loop() {
Serial.write('a');
delay(10);
return;
  	char c = 0x00;
  // put your main code here, to run repeatedly: 
  while(1){
	  Serial.write(c++);
  }
  delay(100);
}
