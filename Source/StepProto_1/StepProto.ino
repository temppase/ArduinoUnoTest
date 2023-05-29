/*
Step motor prototype. Some features still missing or need to set.
*/
// Setups...
#define CIRCLE_MM 40
#define INTERVAL_LENGHT_MM 100
#define INTERVALS 7
#define WAIT_S 2
#define MAX_MM 1000
#define OFFSET 0
#define CAMERA_T 10

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}
bool direction = 1;
int max = MAX_MM;
int interval = interval_steps(CIRCLE_MM, INTERVAL_LENGHT_MM);
int t = 0;
int move = 0;
int moved_steps = 0;
int ic = 0;
// the loop function runs over and over again forever
void loop() {
  if(direction == 1){
    while(max > 0 && ic < INTERVALS){
      picture();
      testprint();
      
      move = move + turn(interval);
      max = max - INTERVAL_LENGHT_MM;
      t++;
      ic++;
      delay(1000 * WAIT_S - (CAMERA_T * 3));
    }
    direction = 0;
  }else{
    gohome();
    exit(0);
  }


}
int interval_steps(double c, double i){
  double sl = c / 200;
  return i / sl;
}
int turn(int i){
  int j = 0;
  while(i > 0){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(CAMERA_T/2);
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(CAMERA_T/2);
    i--;
    j++;
    moved_steps++;
  }
  return INTERVAL_LENGHT_MM;
}
void gohome(){
  while(moved_steps >= 0){
    if(moved_steps % 100 == 0){
      Serial.println("|");
    }else{
      Serial.print("|");
    }
    moved_steps--;
  }
  delay(500);
}
void picture(){
  Serial.println("Click!...");
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(CAMERA_T);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(CAMERA_T);
}
void testprint(){
  Serial.print("Turn: ");
  Serial.print(t);
  Serial.print(". ");
  Serial.print("move = ");
  Serial.print(move);
  Serial.print(" mm, steps = ");
  Serial.println(moved_steps);

}