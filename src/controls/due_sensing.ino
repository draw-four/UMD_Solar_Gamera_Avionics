String readString;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
}

void loop() {
  /*if (Serial1.available()) {
   int inByte = Serial1.read();
   Serial.println(inByte);
  }*/
  while (Serial1.available()) {
    delay(3);  //delay to allow buffer to fill 
    if (Serial1.available() >0) {
      char c = Serial1.read();  //gets one byte from serial buffer
      readString += c; //makes the string readString
    } 
    Serial.print(readString);
    readString = "";
  }
// readString may then be passed to the atoi() function to be converted to an integer!
// potential problems are variable length integers (90 and 102, maybe have hall sensor
// pass in some kind of delimiting char). If we get that fixed, we may store the latest ones (end of string) in a buffer system
}
