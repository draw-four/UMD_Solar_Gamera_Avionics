/*
BLDC controller based on keyboard input
*/

#include <Servo.h> // <--------------------------- possibly a missing file? I would like to see it
// #include "LowPower.h" <------------------------ may be relevant

#define MIN_SIGNAL 800
#define MAX_SIGNAL 2200

// read key vars
char key;
const int BUFFER_SIZE = 1;
char buf[2];

String readString, readString2, readString3;
const int SERIAL_BUFFER_SIZE = 3;
char serial_buf[3];

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
  // Serial4.begin(9600); remember that the fourth serial is basically the original Serial
  
  Serial.setTimeout(20);  

  Serial1.setTimeout(20);
  Serial2.setTimeout(20);
  Serial3.setTimeout(20);
  
  pinMode(13, OUTPUT);

  // initialize motor
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.attach(MOTOR_PIN[i]);
    allMotors[i].power = MIN_SIGNAL;
    allMotors[i].motor.writeMicroseconds(MIN_SIGNAL);
  }

  // sync with main
  Serial.print(1);

  // wait for prompt to start 
  while (buf[1] != '\t')
      Serial.readBytes(buf, 2);
  buf[1] = ' ';

  // calibrate if required
  if (buf[0] == 'c')
    calibrate(); 

  // begin program
  Serial.println(0);
  // flash LED x5
  for(int i = 0; i < 5; i++) {
    digitalWrite(13,HIGH); 
    delay(100);
    digitalWrite(13,LOW);
    delay(100);
  }
}

// loop
void loop() {
  // reading key takes total of 20ms (based on timeout)
  if (Serial.available()){
    
  Serial.readBytes(buf, BUFFER_SIZE);
  key = buf[0];

  switch (key) {
    // increase total throttle
    case 'H':
      for (int i = 0; i < 4; i++) {
        allMotors[i].power += 5;
        allMotors[i].power >= MAX_SIGNAL ? allMotors[i].power = MAX_SIGNAL : allMotors[i].power = allMotors[i].power; // saturate
      }
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // decrease total throttle
    case 'P':
      for (int i = 0; i < 4; i++) {
        allMotors[i].power -= 5;
        allMotors[i].power <= MIN_SIGNAL ? allMotors[i].power = MIN_SIGNAL : allMotors[i].power = allMotors[i].power; // saturate
      }
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // individual motor control => increase
    // blue
    case 'w': // <----------------------------------------- wasd may be refactored to use joystick instead
      allMotors[0].power += 5;
      allMotors[0].power >= MAX_SIGNAL ? allMotors[0].power = MAX_SIGNAL : allMotors[0].power = allMotors[0].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // red
    case 'a':
      allMotors[1].power += 5;
      allMotors[1].power >= MAX_SIGNAL ? allMotors[1].power = MAX_SIGNAL : allMotors[1].power = allMotors[1].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // yellow
    case 's':
      allMotors[2].power += 5;
      allMotors[2].power >= MAX_SIGNAL ? allMotors[2].power = MAX_SIGNAL : allMotors[2].power = allMotors[2].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // pink
    case 'd':
      allMotors[3].power += 5;
      allMotors[3].power >= MAX_SIGNAL ? allMotors[3].power = MAX_SIGNAL : allMotors[3].power = allMotors[3].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // decrease
    // blue
    case 'W':
      allMotors[0].power -= 5;
      allMotors[0].power <= MIN_SIGNAL ? allMotors[0].power = MIN_SIGNAL : allMotors[0].power = allMotors[0].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // red
    case 'A':
      allMotors[1].power -= 5;
      allMotors[1].power <= MIN_SIGNAL ? allMotors[1].power = MIN_SIGNAL : allMotors[1].power = allMotors[1].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // yellow
    case 'S':
      allMotors[2].power -= 5;
      allMotors[2].power <= MIN_SIGNAL ? allMotors[2].power = MIN_SIGNAL : allMotors[2].power = allMotors[2].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // pink
    case 'D':
      allMotors[3].power -= 5;
      allMotors[3].power <= MIN_SIGNAL ? allMotors[3].power = MIN_SIGNAL : allMotors[3].power = allMotors[3].power; // saturate
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // quit program
    case 'Q':
      quit();
  }
  }

  while (Serial1.available()) {
    delay(3);  //delay to allow buffer to fill 
    if (Serial1.available() >0) {
      char c = Serial1.read();  //gets one byte from serial buffer
      readString += c; //makes the string readString
    } 
    /*
    Serial1.readBytes(serial_buf, SERIAL_BUFFER_SIZE);
    readString = serial_buf[2] + serial_buf[1] + serial_buf[0];
    */
    Serial.print("RPM1: " + readString);
    readString = "";
  }

  while (Serial2.available()) {
    delay(3);  //delay to allow buffer to fill 
    if (Serial2.available() >0) {
      char c = Serial2.read();  //gets one byte from serial buffer
      readString2 += c; //makes the string readString
    } 
    /*
    Serial2.readBytes(serial_buf, SERIAL_BUFFER_SIZE);
    readString2 = serial_buf[2] + serial_buf[1] + serial_buf[0];
    */
    Serial.print("RPM2: " + readString2);
    readString2 = "";
  }

  while (Serial3.available()) {
    delay(3);  //delay to allow buffer to fill 
    if (Serial3.available() >0) {
      char c = Serial3.read();  //gets one byte from serial buffer
      readString3 += c; //makes the string readString
    } 
    /*
    Serial3.readBytes(serial_buf, SERIAL_BUFFER_SIZE);
    readString3 = serial_buf[2] + serial_buf[1] + serial_buf[0];
    */
    Serial.print("RPM3: " + readString3);
    readString3 = "";
  }

  delay(5);
  // update throttle
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.writeMicroseconds(allMotors[i].power);
  }  
  digitalWrite(13,LOW);
}

// ESC calibration routine
void calibrate() {
  // send max throttle
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.writeMicroseconds(MAX_SIGNAL);
  }

  // calibration begin, plug in power here
  // represented by 2
  Serial.println(2);

  // flash LED as indication
  for(int i = 0; i < 3; i++) {
    digitalWrite(13,HIGH); 
    delay(1000);
    digitalWrite(13,LOW);
    delay(1000);
  }

  // wait for tone to end then input
  while (buf[1] != '\t')
      Serial.readBytes(buf, 2);
  buf[1] = ' ';

  // sending min throttle
  // represented by 3
  Serial.println(3);
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.writeMicroseconds(MIN_SIGNAL);
  }
  // flash LED to indicate completion
  for(int i = 0; i < 3; i++) {
    digitalWrite(13,HIGH); 
    delay(1000);
    digitalWrite(13,LOW);
    delay(1000);
  }

  // power cycle now

  // wait for prompt to start
  while (buf[1] != '\t')
    Serial.readBytes(buf, 2);
}

void quit() {
  Serial.println(4);

  // LED 
  for (int i = 200; i > 0; i--) {
    float DC = i / 200.0;
    digitalWrite(13, 1);
    delayMicroseconds(floor(2000 * DC));
    digitalWrite(13, 0);
    delayMicroseconds(floor(2000 * (1 - DC)));
  }

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
  Serial.println(5);
  for (int i = 0; i < 4; i++) { 
    allMotors[i].motor.detach();
  }
  Serial.end();

  // sleep forever until restart
  // LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void displayAllMotors() {
  for (int i = 0; i < 4; i++) {
    Serial.print(allMotors[i].power);
    Serial.print('\t');
  }
  Serial.print('\n');
}
