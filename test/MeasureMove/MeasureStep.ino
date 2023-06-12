/*Stepper motor measuring program*/
// Include library
#include <AccelStepper.h>
// Define stepper motor connections
#define dirPin 2
#define stepPin 3
#define motorinterfaceType 1
#define S 1000
#define M 60000

// Create instabce of the AccelStepper class
AccelStepper stepper = AccelStepper(motorinterfaceType, stepPin, dirPin);
void setup() {
  // Set max speed in step per seconds
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(30);
  Serial.begin(9600);
}
// MAIN loop!
void loop() {
  // move 100 steps and measure
  stepper.currentPosition(0);
  stepper.moveTo(100);
  stepper.runToPosition();
  Serial.println("Ready to measure.");
  delay(M);
  stepper.moveTo(0);
  stepper.runToPosition();
  delay(S);
  exit(0);
}
