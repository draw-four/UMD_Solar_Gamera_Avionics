/*
BLDC controller based on keyboard input
*/

/******* MOTOR CONTROL VARIABLES *******/

#include <Servo.h>
#include <SoftwareSerial.h>
// #include "LowPower.h" // may be relevant? <------------------------------------------------------------------------------

#define MIN_SIGNAL 800
#define MAX_SIGNAL 2200

// read key vars
char key;
const int BUFFER_SIZE = 1;
const int WINDOW_SIZE = 3;
char buf[2];

/*************************************/

/******* HALL SENSOR VARIABLES *******/ // <--------------------------refactor so each motor contains its own cnt, start, and RPM if testing works

const int NUMBER_OF_MOTORS = 4;
const int hall_pin_arr[NUMBER_OF_MOTORS] = {2,3,4,5};
const int maxCnt = 1;
unsigned long start_arr[NUMBER_OF_MOTORS];

// volatile because it depends on user input and measurement delays
// unsure if volatile int array[...] would break anything <------------------------------------------------------------------------------------
// if it does replace with the old:
// volatile int cnt0 = 0;
// volatile int cnt1 = 0;
// volatile int cnt2 = 0;
// volatile int cnt3 = 0;
volatile int cnt_arr[NUMBER_OF_MOTORS] = {0,0,0,0};

unsigned long start_array[NUMBER_OF_MOTORS] = {0,0,0,0};

int RPM[NUMBER_OF_MOTORS] = {0};

int minimum_rpm = 0;

/*************************************/

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

/******* HALL SENSOR FUNCTIONS *******/
void count0() {
  cnt_arr[0]++;
}

void count1() {
  cnt_arr[1]++;
}

void count2() {
  cnt_arr[2]++;
}

void count3() {
  cnt_arr[3]++;
}

int sense_rpm(MOTOR motor, int motor_num) {
  int motor_cnt = 0;
  int start = 0;
  switch(motor_num){
    case 0: motor_cnt = cnt_arr[0]; start = start_arr[0]; break;
    case 1: motor_cnt = cnt_arr[1]; start = start_arr[1]; break;
    case 2: motor_cnt = cnt_arr[2]; start = start_arr[2]; break;
    case 3: motor_cnt = cnt_arr[3]; start = start_arr[3]; break;
    default: break;
  } 

  if (motor_cnt > maxCnt) {
    float seconds = (micros() - start) / 1000000.0; // implemented with start_array; may be a problem if taken out of loop
    float rpm = (motor_cnt / seconds * 60.0) / 3;

    motor.SUM -= motor.READINGS[motor.INDEX];       // Remove the oldest entry from the sum
    motor.VALUE = rpm;        // Read the next sensor value
    motor.READINGS[motor.INDEX] = motor.VALUE;           // Add the newest reading to the window
    motor.SUM += motor.VALUE;                 // Add the newest reading to the sum
    motor.INDEX = (motor.INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

    motor.AVERAGED = motor.SUM / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

    switch(motor_num){
      case 0: cnt_arr[0] = 0; start_arr[0] = micros(); break;
      case 1: cnt_arr[1] = 0; start_arr[1] = micros(); break;
      case 2: cnt_arr[2] = 0; start_arr[2] = micros(); break;
      case 3: cnt_arr[3] = 0; start_arr[3] = micros(); break;
      default: break;
    }
  }
  return motor.AVERAGED;
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
  // if code does not work CHECK HERE! <---------------------------------------------------------------------------------------------------------------------
  const int unused_tx = 13;
  SoftwareSerial serial_0 = SoftwareSerial(hall_pin_arr[0], unused_tx);
  SoftwareSerial serial_1 = SoftwareSerial(hall_pin_arr[1], unused_tx);
  SoftwareSerial serial_2 = SoftwareSerial(hall_pin_arr[2], unused_tx);
  SoftwareSerial serial_3 = SoftwareSerial(hall_pin_arr[3], unused_tx);
  pinMode(unused_tx, OUTPUT);
  serial_0.begin(9600);
  serial_1.begin(9600);
  serial_2.begin(9600);
  serial_3.begin(9600);

  pinMode(hall_pin_arr[0], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[0]), count0, FALLING);
  pinMode(hall_pin_arr[1], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[1]), count1, FALLING);
  pinMode(hall_pin_arr[2], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[2]), count2, FALLING);
  pinMode(hall_pin_arr[3], INPUT);
  attachInterrupt(digitalPinToInterrupt(hall_pin_arr[3]), count3, FALLING);

  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
    start_array[i] = micros();
  }

  // needs to print 1 to interface with keyboard controller script
  Serial.print(1);

  }

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

      /******* AUTO STABILIZING CODE *******/
  
      /* set up to loop every run*/
      Serial.print("  Detected RPM: ");
      for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
        RPM[i] = sense_rpm(allMotors[i], i);
        Serial.print(RPM[i]);
        Serial.print('\t');
        /*
          MAIN PROBLEM: IF RPM IS LIKE 10-20 TIMES A SECOND AND MOTORS ARE RUNNING AT POWER >1000, AND RPMS ARE MARKEDLY DIFFERENT, THERE MAY BE NO WAY TO DIRECTLY ADJUST.
          WE MAY NEED TO PICK ONE RPM (SMALLEST OR LARGEST RPM?) AND FORCE OTHERS TO THAT RPM.
          FOR THE TEST, FIRST CHECK RPM. PERHAPS WE *CAN* ESTABLISH SOME RELATIONSHIP BETWEEN POWER AND RPM WITH ACTUAL BLADE TEST.
          ALSO THE BELOW CODE DOES NOT WORK EVEN IF IT WAS 1 TO 1, as both RPM and .power would chase each other to saturation. 
          ALSO TEST CODE TRIES TO BALANCE IN ALL CONDITIONS, A PERMANENT HOVER MODE (PRESSING W WOULD BRING UP ASD AS WELL)
        */
        // LEGACY BAD CODE:
        // int correction = RPM - allMotors[i].power; // note that upping the power and the measured RPM IS NOT A ONE TO ONE! EXPERIMENT <-------------------
        // (correction > 10 && RPM < MAX_SIGNAL) ? allMotors[i].power = RPM : allMotors[i].power = allMotors[i].power; // saturate, positive
        // (correction < -10 && RPM > MIN_SIGNAL) ? allMotors[i].power = RPM : allMotors[i].power = allMotors[i].power; // saturate, negative
        // END LEGACY CODE

        
      // }
      // TEST CODE
      // maximum_rpm = RPM[0];
      // for (int i = 1; i < NUMBER_OF_MOTORS; i++){
      //   if (RPM[i] > maximum_rpm) {
      //     maximum_rpm = RPM[i];
      //   }
      // }
      // for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
      //   if (maximum_rpm - RPM[i] >= 0) { // change as needed
      //     allMotors[i].power++; // small +1 increments so as to not overwhelm speed adjustments
      //   }
      // }
      // Serial.print('\n');
      /***********************************/
      }
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
