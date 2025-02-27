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
const int WINDOW_SIZE = 3;
char buf[2];

// motor struct
const int MOTOR_PIN[NUMBER_OF_MOTORS] = {6,9,10,11};
struct MOTOR{
  Servo motor;
  int power;
  int INDEX = 0;
  float VALUE = 0;
  float SUM = 0;
  float READINGS[WINDOW_SIZE];
  int AVERAGED = 0;
};
struct MOTOR allMotors[NUMBER_OF_MOTORS];

// function prototypes
void calibrate();
void quit();
void displayAllMotors();
void count(int motor_num);
int sense_rpm(MOTOR motor);

/*************************************/

/******* HALL SENSOR VARIABLES *******/

const int NUMBER_OF_MOTORS = 4; // REPLACED ALL INSTANCES OF 4 WITH THIS CONSTANT FOR TESTING ! <---------------------------------------------------
const int hall_pin_arr[NUMBER_OF_MOTORS] = {2,3,4,5};
const int maxCnt = 1;

// volatile because it depends on user input and measurement delays
// unsure if volatile int array[...] would break anything <------------------------------------------------------------------------------------
// if it does replace with the old:
// volatile int cnt0 = 0;
// volatile int cnt1 = 0;
// volatile int cnt2 = 0;
// volatile int cnt3 = 0;
(volatile int) cnt_arr[NUMBER_OF_MOTORS] = {0}; // or {0,0,0,0}

/*************************************/

/******* HALL SENSOR FUNCTIONS *******/
void count(int motor_num) {
  switch (motor_num) {
    case 0: cnt_arr[0]++; break;
    case 1: cnt_arr[1]++; break;
    case 2: cnt_arr[2]++; break;
    case 3: cnt_arr[3]++; break;
    default: break;
  })
}

int sense_rpm(MOTOR motor) {
  int motor_cnt = 0;
  switch(motor_num){
    case 0: motor_cnt = cnt_arr[0]; start = start_arr[0]; break;
    case 1: motor_cnt = cnt_arr[1]; start = start_arr[1]; break;
    case 2: motor_cnt = cnt_arr[2]; start = start_arr[2]; break;
    case 3: motor_cnt = cnt_arr[3]; start = start_arr[3]; break;
    default: break;
  } 

  // move back here
  if (motor_cnt > maxCnt) {
    float seconds = (micros() - start) / 1000000.0;
    float rpm = (motor_cnt / seconds * 60.0) / 3;

    motor->SUM -= motor->READINGS[INDEX];       // Remove the oldest entry from the sum
    motor->VALUE = rpm;        // Read the next sensor value
    motor->READINGS[INDEX] = motor->VALUE;           // Add the newest reading to the window
    motor->SUM += motor->VALUE;                 // Add the newest reading to the sum
    motor->INDEX = (motor->INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

    motor->AVERAGED = motor->SUM / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

    switch(motor_num){
      case 0: cnt_arr[0] = 0; start_arr[0] = micros(); break;
      case 1: cnt_arr[1] = 0; start_arr[1] = micros(); break;
      case 2: cnt_arr[2] = 0; start_arr[2] = micros(); break;
      case 3: cnt_arr[3] = 0; start_arr[3] = micros(); break;
      default: break;
    }
  }
  return motor->AVERAGED;
}

/*************************************/

// setup 
void setup() {
  // general config
  Serial.begin(115200);
  Serial.setTimeout(20);  
  pinMode(13, OUTPUT); // LED

  // initialize motor
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
    allMotors[i].motor.attach(MOTOR_PIN[i]);
    allMotors[i].power = MIN_SIGNAL;
    allMotors[i].motor.writeMicroseconds(MIN_SIGNAL);

  // initialize HALL SENSOR

  // Serial.begin(9600);
  // hopefully sensor works on the same serial port, if not may need to change the mainbaud rate/a second port! 
  // if code does not work CHECK HERE! <---------------------------------------------------------------------------------------
  pinMode(hall_pin_arr[0], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[0]), count(0), FALLING);
  pinMode(hall_pin_arr[1], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[1]), count(1), FALLING);
  pinMode(hall_pin_arr[2], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[2]), count(2), FALLING);
  pinMode(hall_pin_arr[3], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[3]), count(3), FALLING);

  unsigned long start_array[4] = {micros()}; // refactored to include all hall sensor function here! <-------------------------------------------------

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
      for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
        allMotors[i].power += 5;
        allMotors[i].power >= MAX_SIGNAL ? allMotors[i].power = MAX_SIGNAL : allMotors[i].power = allMotors[i].power; // saturate
      }
      displayAllMotors();
      digitalWrite(13,HIGH);  
      break;

    // decrease total throttle
    case 'P':
      for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
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

    // quit program
    case 'Q':
      quit();
  }

  /******* AUTO STABLIZING CODE *******/
  /* set up to loop every run*/
  Serial.print("Detected RPM: ")
      for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
        int RPM = sense_rpm(allMotors[i]);
        Serial.print(RPM);
        Serial.print('\t');
        int correction = RPM - allMotors[i].power; // note that upping the power and the measured RPM IS NOT A ONE TO ONE! EXPERIMENT <-------------------
        (correction > 10 && RPM < MAX_SIGNAL) ? allMotors[i].power = RPM : allMotors[i].power = allMotors[i].power; // saturate, positive
        (correction < -10 && RPM > MIN_SIGNAL) ? allMotors[i].power = RPM : allMotors[i].power = allMotors[i].power; // saturate, negative
      }
      Serial.print('\n');
  /***********************************/

  delay(5);
  // update throttle
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
    allMotors[i].motor.writeMicroseconds(allMotors[i].power);
  }  
  digitalWrite(13,LOW);
}

// ESC calibration routine
void calibrate() {
  // send max throttle
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
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
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
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
  int dp[NUMBER_OF_MOTORS];
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) { 
    dp[i]= allMotors[i].power - MIN_SIGNAL;
  }

  for (int i = 0; i <= 100; i++) {
    for (int i = 0; i < NUMBER_OF_MOTORS; i++) { 
      allMotors[i].power -= dp[i]/100;
      allMotors[i].motor.writeMicroseconds(allMotors[i].power);
    }
    delay(50);
  }

  // detach motors and Serial just in case
  Serial.println(5);
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) { 
    allMotors[i].motor.detach();
  }
  Serial.end();

  // sleep forever until restart
  // LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); <-----------------------------------------------------------------------

  /*************************************/
}

void displayAllMotors() {
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
    Serial.print(allMotors[i].power);
    Serial.print('\t');
  }
  Serial.print('\n');
}
