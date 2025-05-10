/*
BLDC controller based on ARDUINO SERIAL MONITOR INPUT
Removed all bufs, readBytes, and print(int)s. 
*/

#include <Servo.h>
// #include <SoftwareSerial.h>
// SOFTWARE SERIAL NOT AVAILABLE IN DUE!
// const byte rxPin = 10;
// const byte txPin = 11;
// SoftwareSerial soft_serial (rxPin, txPin);

#define MIN_SIGNAL 800
#define MAX_SIGNAL 2200

// read key vars
char key;
float rpm_1, rpm_2, rpm_3, rpm_4;

// motor struct
const int MOTOR_PIN[4] = {6,9,10,11};
struct MOTOR{
  Servo motor;
  int power;
};
struct MOTOR allMotors[4];

// function prototypes
void calibrate();
void quit();
void displayAllMotors();

// setup 
void setup() {
  // general config
  Serial.begin(115200);

  Serial1.begin(9600); // make sure the pins for these are NOT rx and tx 0 and 1
  Serial2.begin(9600);
  Serial3.begin(9600);
  //soft_serial.begin(9600); // the fourth one

  
  Serial.setTimeout(20);  

  Serial1.setTimeout(20);
  Serial2.setTimeout(20);
  Serial3.setTimeout(20);
  //soft_serial.setTimeout(20);
  

  // initialize motor
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.attach(MOTOR_PIN[i]);
    allMotors[i].power = MIN_SIGNAL;
    allMotors[i].motor.writeMicroseconds(MIN_SIGNAL);
  }

  // this will sometimes double print, blame poor Arduino scheduling
  Serial.println("Press c to calibrate, else press any key to continue: ");
  while (Serial.available() == 0) ;
  key = Serial.read();
  // calibrate if required
  if (key == 'c') {
    calibrate(); 
  }
  Serial.println("Program begins.");
}

// loop
void loop() {
  if (Serial.available() > 0){ // if any key is detected from the keyboard
  
  key = Serial.read(); // read in one character
  // Serial.print(key);
  switch (key) {
    // increase total throttle
    case 'H':
      for (int i = 0; i < 4; i++) {
        allMotors[i].power += 5;
        allMotors[i].power >= MAX_SIGNAL ? allMotors[i].power = MAX_SIGNAL : allMotors[i].power = allMotors[i].power; // saturate
      }
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // decrease total throttle
    case 'P':
      for (int i = 0; i < 4; i++) {
        allMotors[i].power -= 5;
        allMotors[i].power <= MIN_SIGNAL ? allMotors[i].power = MIN_SIGNAL : allMotors[i].power = allMotors[i].power; // saturate
      }
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // individual motor control => increase
    // blue
    case 'w': // <----------------------------------------- wasd may be refactored to use joystick instead
      allMotors[0].power += 5;
      allMotors[0].power >= MAX_SIGNAL ? allMotors[0].power = MAX_SIGNAL : allMotors[0].power = allMotors[0].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // red
    case 'a':
      allMotors[1].power += 5;
      allMotors[1].power >= MAX_SIGNAL ? allMotors[1].power = MAX_SIGNAL : allMotors[1].power = allMotors[1].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // yellow
    case 's':
      allMotors[2].power += 5;
      allMotors[2].power >= MAX_SIGNAL ? allMotors[2].power = MAX_SIGNAL : allMotors[2].power = allMotors[2].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // pink
    case 'd':
      allMotors[3].power += 5;
      allMotors[3].power >= MAX_SIGNAL ? allMotors[3].power = MAX_SIGNAL : allMotors[3].power = allMotors[3].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // decrease
    // blue
    case 'W':
      allMotors[0].power -= 5;
      allMotors[0].power <= MIN_SIGNAL ? allMotors[0].power = MIN_SIGNAL : allMotors[0].power = allMotors[0].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // red
    case 'A':
      allMotors[1].power -= 5;
      allMotors[1].power <= MIN_SIGNAL ? allMotors[1].power = MIN_SIGNAL : allMotors[1].power = allMotors[1].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // yellow
    case 'S':
      allMotors[2].power -= 5;
      allMotors[2].power <= MIN_SIGNAL ? allMotors[2].power = MIN_SIGNAL : allMotors[2].power = allMotors[2].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // pink
    case 'D':
      allMotors[3].power -= 5;
      allMotors[3].power <= MIN_SIGNAL ? allMotors[3].power = MIN_SIGNAL : allMotors[3].power = allMotors[3].power; // saturate
      displayAllMotors();
      // digitalWrite(13,HIGH);  
      break;

    // quit program
    case 'Q':
      quit(); // this function works properly
      break;

    case 'U':
      while (Serial.available()) { // if there is a new RPM measurement
        delay(3);  // delay to allow buffer to fill 
        if (Serial.available() >0) {
        rpm_4 = Serial.parseFloat(); // make sure the No Line Ending is set for all Serial Monitors just in case.
        }
      Serial.print("RPM4: ");
      Serial.println(rpm_4);
      }
      break;
      
    }
    
  }

  // RPM display prototype. 
  while (Serial1.available()) { // if there is a new RPM measurement
    delay(3);  // delay to allow buffer to fill 
    if (Serial1.available() >0) {
      rpm_1 = Serial1.parseFloat(); // make sure the No Line Ending is set for all Serial Monitors just in case.
    }
      Serial.print("RPM1: ");
      Serial.println(rpm_1);
  }

 while (Serial2.available()) { // if there is a new RPM measurement
    delay(3);  // delay to allow buffer to fill 
    if (Serial2.available() >0) {
      rpm_2 = Serial2.parseFloat(); // make sure the No Line Ending is set for all Serial Monitors just in case.
    }
      Serial.print("RPM2: ");
      Serial.println(rpm_2);
  }

  while (Serial3.available()) { // if there is a new RPM measurement
    delay(3);  // delay to allow buffer to fill 
    if (Serial3.available() >0) {
      rpm_3 = Serial3.parseFloat(); // make sure the No Line Ending is set for all Serial Monitors just in case.
    }
      Serial.print("RPM3: ");
      Serial.println(rpm_3);
  }

/*
  while (soft_serial.available()) { // if there is a new RPM measurement
    delay(3);  // delay to allow buffer to fill 
    if (soft_serial.available() >0) {
      rpm_4 = soft_serial.parseFloat(); // make sure the No Line Ending is set for all Serial Monitors just in case.
    }
      soft_serial.print("RPM4: ");
      soft_serial.println(rpm_4);
  }
  */


  delay(5);
  // update throttle
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.writeMicroseconds(allMotors[i].power);
  }  
  // digitalWrite(13,LOW);
}

// ESC calibration routine
void calibrate() {
  /*
    THE CALIBRATE FUNCTION WORKS DIFFERENTLY
    SIMPLY WALKS YOU THRU THE POWER CYCLE PROCESS
  */

  // no longer sending max throttle

  Serial.println("Calibration begins. ");

  /*
    SHOULD BEEP HERE UPON TURNING ON
    ORIGINAL SPEED CONTROLLER: *monotone* beep beep
    NEW SPEED CONTROLLER: *rising tone* beep beep beep *pause* *monotone* beep beep *pause* beeeeeeep
  */
  
  // wait for tone to end then input
  Serial.println("Power cycle after tone, then press any key.");
  while (Serial.available() == 0) ;
  while(Serial.available() != 0 ) {
    Serial.read();
  }
  /*
    SHOULD BEEP AGAIN HERE UPON POWER CYCLE
  */

  // sending min throttle for legacy's sake lol
  Serial.println("Sending min throttle. ");
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.writeMicroseconds(MIN_SIGNAL);
  }

  // wait for prompt to start
delay(1000);
Serial.println("Press any key to begin. ");
while (Serial.available() == 0) ;
while(Serial.available() != 0 ) {
    Serial.read();
}

}

void quit() {
  Serial.println("Quitting program...");

  // decelerating motors
  int dp[4];
  for (int i = 0; i < 4; i++) { 
    dp[i]= allMotors[i].power - MIN_SIGNAL;
  }

  for (int i = 0; i <= 100; i++) {
    for (int i = 0; i < 4; i++) { 
      allMotors[i].power -= dp[i]/100;
      allMotors[i].motor.writeMicroseconds(allMotors[i].power);
    }
    delay(50);
  }

  // detach motors and Serial just in case
  Serial.println("Detaching motors. ");
  for (int i = 0; i < 4; i++) { 
    allMotors[i].motor.detach();
  }
  Serial.println("Successfully quit. ");
  Serial.end();
  while (1) ;

  // sleep forever until restart
}

void displayAllMotors() {
  for (int i = 0; i < 4; i++) {
    Serial.print(allMotors[i].power);
    Serial.print('\t');
  }
  Serial.print('\n');
}
