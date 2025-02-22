/*
BLDC controller based on keyboard input
*/

/******* MOTOR CONTROL VARIABLES *******/

#include <Servo.h>
// #include "LowPower.h" // may be relevant? <------------------------------------------------------------------------------

#define MIN_SIGNAL 800
#define MAX_SIGNAL 2200

// read key vars
char key;
const int BUFFER_SIZE = 1;
char buf[2];

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

/*************************************/

/******* HALL SENSOR VARIABLES *******/

const int hallPin0 = 2;
// are we connecting four hall sensors to the same board? do we have enough analog ports on the nano for that? <----------------------
const int hallPin1 = 3;
const int hallPin2 = 4;
const int hallPin3 = 5;
const int maxCnt = 1;

volatile int cnt0 = 0; // volatile because it depends on user input and measurement delays
volatile int cnt1 = 0;
volatile int cnt2 = 0;
volatile int cnt3 = 0;

int INDEX = 0;
float VALUE = 0;
float SUM = 0;
const int WINDOW_SIZE = 3;
float READINGS[WINDOW_SIZE];
int AVERAGED = 0;

/*************************************/

/******* HALL SENSOR FUNCTIONS *******/
// switch statements are fast but if the sensor seems to lag maybe make four separate functions instead? <--------------------------------------
void count(int motor_num) {
  switch (motor_num) {
    case 0: cnt0++; break;
    case 1: cnt1++; break;
    case 2: cnt2++; break;
    case 3: cnt3++; break;
    default: break;
  })
}

int sense_rpm(int motor_num) {
  int motor_cnt = 0;
  switch(motor_num){
    case 0: motor_cnt = cnt0; break;
    case 1: motor_cnt = cnt1; break;
    case 2: motor_cnt = cnt2; break;
    case 3: motor_cnt = cnt3; break;
    default: break;
  } 

  unsigned long start = micros();
  if (motor_cnt > maxCnt) {
    float seconds = (micros() - start) / 1000000.0;
    float rpm = (motor_cnt / seconds * 60.0) / 3;

    SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum
    VALUE = rpm;        // Read the next sensor value
    READINGS[INDEX] = VALUE;           // Add the newest reading to the window
    SUM = SUM + VALUE;                 // Add the newest reading to the sum
    INDEX = (INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

    AVERAGED = SUM / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

    switch(motor_num){
      case 0: cnt0 = 0; break;
      case 1: cnt1 = 0; break;
      case 2: cnt2 = 0; break;
      case 3: cnt3 = 0; break;
      default: break;
    }
  }
  return AVERAGED;
}

/*************************************/

// setup 
void setup() {
  // general config
  Serial.begin(115200);
  Serial.setTimeout(20);  
  pinMode(13, OUTPUT);

  // initialize motor
  for (int i = 0; i < 4; i++) {
    allMotors[i].motor.attach(MOTOR_PIN[i]);
    allMotors[i].power = MIN_SIGNAL;
    allMotors[i].motor.writeMicroseconds(MIN_SIGNAL);

  // initialize HALL SENSOR

  // Serial.begin(9600);
  // hopefully sensor works on the same serial port, if not may need to change the mainbaud rate/a second port! 
  // if code does not work CHECK HERE! <---------------------------------------------------------------------------------------
  pinMode(hallPin0, INPUT);
  attachInterrupt(digitalPinToInterrupt(hallPin0), count(0), FALLING);
  // pinMode(hallPin1, INPUT);
  // attachInterrupt(digitalPinToInterrupt(hallPin1), count(1), FALLING);
  // pinMode(hallPin2, INPUT);
  // attachInterrupt(digitalPinToInterrupt(hallPin2), count(2), FALLING);
  // pinMode(hallPin3, INPUT);
  // attachInterrupt(digitalPinToInterrupt(hallPin3), count(3), FALLING);
  }

  // sync with main
  Serial.println("Main synced.");

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

/******* MOTOR CODE *******/

  // reading key takes total of 20ms (based on timeout)
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
    case 'w': // wasd may be refactored to use joystick instead <-------------------------------------------------------------------------------
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

    /******* AUTO STABLIZING CODE *******/
    case 'x': // may need to insert case into keyboard_controller.py <------------------------------------------------------------------
      Serial.print("Detected RPM: ")
      for (int i = 0; i < 4; i++) {
        int RPM = sense_rpm(i);
        Serial.print(RPM);
        Serial.print('\t');
        int correction = RPM - allMotors[i].power; // note that upping the power and the measured RPM IS NOT A ONE TO ONE! EXPERIMENT <-------------------
        (correction > 10 && RPM < MAX_SIGNAL) ? allMotors[i].power = RPM : allMotors[i].power = allMotors[i].power; // saturate, positive
        (correction < -10 && RPM > MIN_SIGNAL) ? allMotors[i].power = RPM : allMotors[i].power = allMotors[i].power; // saturate, negative
      }
      Serial.print('\n');
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;
    /***********************************/

    // quit program
    case 'Q':
      quit();
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
  // LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); <-----------------------------------------------------------------------

  /*************************************/
}

void displayAllMotors() {
  for (int i = 0; i < 4; i++) {
    Serial.print(allMotors[i].power);
    Serial.print('\t');
  }
  Serial.print('\n');
}
