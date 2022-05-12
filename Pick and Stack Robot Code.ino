
#include "SR04.h"
#include <Servo.h>
#include <Stepper.h>

// ultrasonic sensor i/o pins
#define TRIG_PIN1 10
#define ECHO_PIN1 11
#define TRIG_PIN2 12
#define ECHO_PIN2 13

// servo i/o pins
int BASE_PIN = 6;
int LOWER_ARM_PIN = 7;
int UPPER_ARM_PIN = 8;
int TOP_PLATE_PIN = 9;

// use i/o pins 5-8 for stepper motor
const int stepsPerRevolution = 2048;
const int rpm = 15;
Stepper stepper(stepsPerRevolution, 4, 2, 5, 3);

// servos 
Servo base;
Servo lower_arm;
Servo upper_arm;
Servo top_plate; 

// ultrasonic sensors
SR04 ultrasonic_ref = SR04(ECHO_PIN1,TRIG_PIN1);
SR04 ultrasonic_gnd = SR04(ECHO_PIN2,TRIG_PIN2);
long a;
long b;

// clamping time 
int SPIN_TIME = 26350;

// absolute default position 
//int default_lower = 100;
//int default_upper = 135;
//int default_top_plate = 140; //160 low - 10 high
//int default_base = 5;

// pick up counter
int num_packages = 0;
int default_pickup = 50;

void moveServo(Servo servo, int angle) {
  int ratio = (1850-500)/180;
  int microsecondsAngle = angle*ratio + 500;
  servo.writeMicroseconds(microsecondsAngle);
}

// testing 
int test_lower = 110;
int test_upper = 175;
int test_top_plate = 50+17; //160 low - 10 high
int test_base = 5;

void testing() {
  delay(2000);
  moveServo(lower_arm, test_lower);
  delay(2000);
  moveServo(upper_arm, test_upper);
  delay(1500);
  top_plate.write(test_top_plate);
  delay(1500);
  base.write(test_base); 
}

void goToDefaultPosition() {
  delay(2000);
  moveServo(lower_arm, 110);
  delay(2000);
  moveServo(upper_arm, 200);
  delay(2000);
  top_plate.write(65+17);
  delay(2000);
  base.write(5);
}

void goToPickupLocation() {
  delay(2000);
  top_plate.write(35+17);
  delay(2000);
  moveServo(lower_arm, 110);
  delay(2000);
  slowMove(upper_arm, 2500, 200, 250);
  delay(2000);
  base.write(5);
  delay(3000);
  //  int pos = default_pickup + num_packages*15;
  //  base.write(pos);
}

void slowMove(Servo servo, int move_time, int startAngle, int stopAngle) {
  unsigned long startTime = millis();
  unsigned long currentTime = millis();

  while(currentTime - startTime <= move_time) {
    long angle = map(currentTime - startTime, 0, move_time, startAngle, stopAngle);
    moveServo(servo, angle);
    currentTime = millis();
  }
}

int getHeight() {
   a = ultrasonic_ref.Distance();
   b = ultrasonic_gnd.Distance();
   long result = abs(a-b);
   Serial.print("A: ");
   Serial.print(a);
   Serial.print(" B: ");
   Serial.println(b);
   Serial.print("RESULT: ");
   Serial.println(result);
   return result;
}

void clampPackage(int SPIN_TIME) {
  unsigned long startTime = millis();
  unsigned long currentTime = millis();

  while(currentTime - startTime <= SPIN_TIME) {
    currentTime = millis();
    stepper.step(stepsPerRevolution);
  }
  delay(2500);
}

void releasePackage(int SPIN_TIME) {
  unsigned long startTime = millis();
  unsigned long currentTime = millis();

  while(currentTime - startTime <= SPIN_TIME) {
    currentTime = millis();
    stepper.step(-stepsPerRevolution);
  }
  delay(2500);
}

int getPile(int height) {
  if (height < 6 && height >= 2) {
    return 1;
  } else if (height >= 6 && height <= 10) {
    return 2;
  } else {
    return 3;
  }
}

int pile_1_pos = 0;
int pile_2_pos = 20;

void goToPile(int pile) {
  if (pile == 1) {
    base.write(pile_1_pos);
  } else if (pile == 2) {
    base.write(pile_2_pos);
  }
}

int height_pile1 = 0;
int height_pile2 = 0;

void stackPackage(int pile, int current_height) {
  int height = height_pile1;
  if (pile == 2) {
    height = height_pile2;
  }
}

void updatePile(int pile, int current_height) {
  // increment the height of the piles
  if (pile == 1) {
    height_pile1 += current_height;
  } else if (pile == 2) {
    height_pile2 += current_height;
  }
}

void attachPins() {
  delay(2000);
  lower_arm.attach(LOWER_ARM_PIN);
  delay(2000);
  upper_arm.attach(UPPER_ARM_PIN);
  delay(2000);
  top_plate.attach(TOP_PLATE_PIN);
  delay(2000);
  base.attach(BASE_PIN);
}

void setup() {
  stepper.setSpeed(rpm);
  Serial.begin(9600);
  // go to default position
  goToDefaultPosition();
  attachPins();
}

void loop() {
  // get height and determine pile placement
  Serial.println("STARTING");
  int currentHeight = getHeight();
  int pile = getPile(currentHeight);
  Serial.print("PILE: ");
  Serial.println(pile);
  Serial.println("start going to pickup 1");
  goToPickupLocation();
  Serial.println("finish pickup 1");
//  if (pile != 3) {
    // activate stepper to hold
    Serial.println("starting clamp 1");
    clampPackage(SPIN_TIME);
    Serial.println("finish clamp 1");
    // go to default position
    Serial.println("go to default 1");
    goToDefaultPosition();
    Serial.println("finish go to default 1");
    delay(10000);
    Serial.println("go back to pick up");
    goToPickupLocation();
    Serial.println("finish going back to pick up location");
    delay(3000);
//    // go to drop off location based on currHeight
//    Serial.println("start going to pickup 2");
//    goToPile(pile);
//    Serial.println("finish going to pickup 2");
//    // go down appropriate height based on packages that came before
//    stackPackage(pile, currentHeight);
//    // update the height of the pile 
    updatePile(pile, currentHeight);
//    // activate stepper to some default release position
    Serial.println("releast package 1");
    releasePackage(SPIN_TIME);
    Serial.println("finish release package 1");
//  }
//  // increment num packages
  num_packages++;
//  // go to default position
  Serial.println("go to default 2");
  goToDefaultPosition();
  Serial.println("finish going back to default 2");
//  // repeat
//  if (num_packages >= 5) {
  exit(0);
//  }
}

/*
#include <Servo.h>

Servo myServo;
unsigned long MOVING_TIME = 3000; // moving time is 3 seconds
unsigned long moveStartTime;
int startAngle = 30; // 30°
int stopAngle  = 90; // 90°

void setup() {
  myServo.attach(9);
  moveStartTime = millis(); // start moving

  // TODO: other code
}

void loop() {
  unsigned long progress = millis() - moveStartTime;

  if (progress <= MOVING_TIME) {
    long angle = map(progress, 0, MOVING_TIME, startAngle, stopAngle);
    myServo.write(angle); 
  }

  // TODO: other code
} */ 
