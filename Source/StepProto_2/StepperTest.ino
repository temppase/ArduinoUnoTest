/*Stepper motor control in intervals*/
#include AccelStepper,h
// Define stepper motor connections and steps per revolution:
#define dirPin 2
#define stepPin 3
#define stepsPerRevolution 200

void setup() {
  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // clockwise fast
  spin(HIGH, 500);
  delay(1000);
  // clockwise slow
  spin(HIGH, 2000);
  delay(1000);
  // counterclockwise fast
  spin(LOW, 500);
  delay(1000);
  // counterclockwise slow
  spin(LOW, 2000);
  //delay(1000);
  exit(0);
}
void spin(int dir, int speed){
  if(dir == 1){
    Serial.print("Direction clockwise, ");
  }else{
    Serial.print("Direction counterclockwise, ");
  }
  Serial.print("delay microseconds: ");
  Serial.println(speed);
  // Set the spinning direction:
  digitalWrite(dirPin, dir);
  // Spin the stepper motor 1 revolution:
  for (int i = 0; i < stepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(speed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(speed);
  }
}
