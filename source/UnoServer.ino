/*
Arduino telnet server stepper motor control
created 2023 Teemu Sekki

 */
#include <ezButton.h>
#include <AccelStepper.h>
#include <SPI.h>
#include <Ethernet.h>
// Globals *****************
#define dirPin 2
#define stepPin 3
#define motorinterfaceType 1
#define limPin 7

ezButton limitSwitch(limPin);
AccelStepper stepper = AccelStepper(motorinterfaceType, dirPin, stepPin);
const double MAX_s = 36000; // 36 cm, 100 steps = 1 mm
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
String command = "";
int qtde;
// telnet defaults to port 23
EthernetServer server(23);
double ready = 0;
int stepsTaken = 0;
bool alreadyConnected = false; // whether or not the client was connected previously
// parameters start ****
double I_mm = 0;
double offset = 0;
double I_s = 0;
double direction = 0;
double I_count = 0;
// parameters end *******
// helppers *************
bool mocon = true; // chenge true when motor connected
bool play = false; // play state
bool CR = false; // command readed
double steps = 0; // measure taken steps
double i = 1;
double start_time = 0;
bool isZero = false; // check the reference point
int limState; 
// Globals end *********************************
void setup() {

  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(300);
  limitSwitch.setDebounceTime(50);

  // initialize the Ethernet device
  Ethernet.begin(mac, ip, myDns, gateway, subnet);

  // Open serial communications for debug if neccessary:
   Serial.begin(9600);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    //Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  // start listening for clients
  server.begin();
}
// ****Loop start****
void loop() {
  checkClient(printData());
      // if command readded true set command
  if(CR == true){setCommand(command);CR = false;}
      // play condition
  if(play){
    // ***Stepper commands***
    steps = I_mm * i;

    // test timer logic..
    if(direction == 1){
      if((millis()/1000) - start_time >= I_s){
        if(steps < MAX_s && i <= I_count){
          i++;
          start_time = millis()/1000;
          if(mocon){
            stepper.moveTo(offset + steps);
            stepper.runToPosition();
          }
          // Serial.print("Interval: ");
          // Serial.print(i - 1);
          // Serial.print(", Run to: ");
          // Serial.print(offset + steps);
          // Serial.print(", time: ");
          // Serial.println(start_time);
        }
        else{
          play = false;
        }
        
      }
    }
    if(direction == -1){
      if((millis()/1000) - start_time >= I_s){
        //Serial.println(steps);
        if(steps < MAX_s && i <= I_count){
          i++;
          start_time = millis()/1000;
          double m = offset - steps;
          if(mocon){
            stepper.moveTo(m);
            stepper.runToPosition();
          }
          // Serial.print("Interval: ");
          // Serial.print(i);
          // Serial.print(" Run to: ");
          // Serial.print(m);
          // Serial.print(", time: ");
          // Serial.println(start_time);
        }
        else{
          play = false;
        }
      }
    }
  }
}
// ******LOOP END*******
String printData(){
  // return info to user
  // add reference point info
  String state = "Connected to: 192.168.1.177:23";
  state += "\n__Values__\n";
  state += "Interval length (mm):";
  state += String(I_mm);
  state += "\nInterval offset (mm):";
  state += String(offset);
  state += "\nInterval time (s):";
  state += String(I_s);
  state += "\nIntervals:";
  state += String(I_count);
  state += "\nDirection:";
  state += String(direction);
  state += "\nPlay:";
  state += String(play);
  state += "\nReference:";
  state += String(isZero);
  return state;
}
void setCommand(String c){

  if(c == "info"){
    // returns only info
    Serial.println("Info");
  }else if(c.indexOf("v") > 0){
    String* p = split(c, '|', qtde);
    double move = p[1].toDouble();
    delete[] p;
    if(mocon){
      stepper.moveTo(move * 100);
      stepper.runToPosition();
      delay(500);
    }
    Serial.print("Move to: ");
    Serial.println(move);
  }else if(c == "play"){
    if(mocon){
      isZero = false;
      toZero();
      //Serial.println(offset);
      stepper.moveTo(offset);
      stepper.runToPosition();
      delay(5);
    }
    play = true;
    start_time = millis()/1000;
  }else if(c == "setref"){
    toZero();
  }else if(c == "reset"){
    Serial.println("Reset");
    play = false;
    I_mm = 0; offset = 0; I_s = 0; I_count = 0; direction = 0; ready = 0; steps = 0;
    Serial.println(I_mm + offset + I_s + I_count + direction + ready + steps);
    if(mocon){
      isZero = false;
      toZero();
    }
  }else{
    // parse values
    //Serial.println(c);
    String* p = split(c, '|', qtde);
    I_count = p[0].toDouble();
    I_mm = p[1].toDouble();
    I_s = p[2].toDouble();
    direction = p[3].toDouble();
    offset = p[4].toDouble() * 100;
    delete[] p;
    Serial.print("Intervals: ");
    Serial.println(I_count);
    Serial.print("Interval steps: ");
    Serial.println(I_mm);
    Serial.print("Interval s: ");
    Serial.println(I_s);
    Serial.print("Dir: ");
    Serial.println(direction);
    Serial.print("Offset: ");
    Serial.println(offset);
  }
}
String* split(String& v, char delimiter, int& length) {
  length = 1;
  bool found = false;
  // Figure out how many itens the array should have
  for (int i = 0; i < v.length(); i++) {
    if (v[i] == delimiter) {
      length++;
      found = true;
    }
  }
  // If the delimiter is found than create the array
  // and split the String
  if (found) {
    // Create array
    String* values = new String[length];
    // Split the string into array
    int i = 0;
    for (int itemIndex = 0; itemIndex < length; itemIndex++) {
      for (; i < v.length(); i++) {

        if (v[i] == delimiter) {
          i++;
          break;
        }
        values[itemIndex] += v[i];
      }
    }
    // Done, return the values
    return values;
  }

  // No delimiter found
  return nullptr;
}
void toZero(){
  limitSwitch.loop();
  limState = limitSwitch.getState();
  // Check if it's already in 0 point
  if(limState == LOW){
    stepper.setCurrentPosition(0);
    Serial.println("Zero point");
    isZero = true;
  }
  else{
    Serial.println("Go to zero!");
    while(stepper.currentPosition() > MAX_s * -1 && digitalRead(limPin) > 0) 
    {
      stepper.setSpeed(-1000);
      stepper.runSpeed();
      delay(2);
    }
    isZero = true;
    Serial.println();
    Serial.println("In zere");
    stepper.setCurrentPosition(0);
  }
}
void checkClient(String m){
  EthernetClient client = server.available();
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer:
      client.flush();
      alreadyConnected = true;
    }
    if(client.available() > 0) {
      // read the string incoming from the client:
        if(m != "Run to reference point"){
          command = client.readStringUntil('*');
        }
        client.println(m);
        CR = true; // command readed true
    }
  }
  client.stop();
}

