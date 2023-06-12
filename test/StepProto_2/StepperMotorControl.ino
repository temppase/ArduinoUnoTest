/*Stepper motor control in intervals*/
// Include library
#include <AccelStepper.h>
// Define stepper motor connections
#define dirPin 2
#define stepPin 3
#define motorinterfaceType 1
double STEPS = 200;
double rps = 5; // Rounds per second
// User setups C_MM circle mm, I_MM interval mm, MAX mm and DELAY seconds.
#define C_MM 40
double I_MM = 70;
#define MAX_MM 1000
#define MAX_I 15
#define DELAY 1
double step = C_MM / STEPS; 
int mm = 0;
int steps = 0;
int i = 0;
// Create instabce of the AccelStepper class
AccelStepper stepper = AccelStepper(motorinterfaceType, stepPin, dirPin);
void setup() {
  // Set max speed in step per seconds
  stepper.setMaxSpeed(rps * STEPS);
  stepper.setAcceleration(30);
  Serial.begin(9600);
}
// MAIN loop!
void loop() {

  Serial.println();
  Serial.println("Set 0");
  stepper.setCurrentPosition(0);
  while(mm < MAX_MM && i <= MAX_I){
    spin();
    mm = mm + I_MM;
    i++;
    delay(DELAY * 1000);
  }
  gohome();
  exit(0);
}
// Rotation function
void spin(){
  steps = steps + (I_MM / step);
  spinPrint();
  stepper.moveTo(steps);
  stepper.runToPosition();
}
// Back home
void gohome(){
    Serial.println("Go home");
    stepper.moveTo(0);
    stepper.runToPosition();
    delay(500);
}
// Info print
void spinPrint(){
  Serial.print("Spin to step: ");
  Serial.print(steps);
  Serial.print(", ");
  Serial.print(mm);
  Serial.println(" mm");
}