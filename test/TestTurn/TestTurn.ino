/*
  TestTurn


*/
// Define used variables here.
#define STEP 200
// DIRECTION 0 anti-clockwise, 1 clockwise.
#define DIRECTION 1
// Move time seconds (0.5 test value) 
#define STEP_TIME 0.5
// the setup function runs once when you press reset or power the board

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}
// Constants:
const double s = 1000;
const double c = 360;


// *****Variables*****

int position = 1;
// Time (t) seconds.
double t = STEP_TIME;
// Picture time milliseconds
double ct = 20;
// Steps
double step = STEP;
bool direction = DIRECTION;
double angle = c / step;
double time = 0;
double turn = 0;
// the loop function runs over and over again forever
void loop() {
  Direction();
  while(step >= 0){
    if(step > 0){
      TakePic();
      Turn();
      Calc();
    }
    else{
      End();
    };
  }
  exit(0);
}
void Direction(){
  if(direction == 0){
    Serial.println("Start anti-clockwise.");
  }
  else{
    Serial.println("Start clockwise.");
  }
}
void TakePic(){
  Serial.print("Camera shot: ");
  Serial.print(position);
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(ct);                      // wait for a camera shot
}
void Turn(){
  Serial.print(", Turn: ");
  Serial.print(turn);
  Serial.print(", Time: ");
  Serial.print(time / s);
  Serial.println(" s.");
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(t * s - ct);                    // turning time
}
void Calc(){
  position++;
  // Fix position time relation by -1.
  time = t * (position - 1) * s;
  step--;
  turn = turn + angle;
}
void End(){
  Serial.println("Complete! Return to zero.");
  delay(500);
  step--;
}
