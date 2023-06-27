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
<<<<<<< HEAD
AccelStepper stepper = AccelStepper(motorinterfaceType, dirPin, stepPin);
const double MAX_s = 35000; // 36 cm, 100 steps = 1 mm
=======
AccelStepper stepper = AccelStepper(motorinterfaceType, stepPin, dirPin);
const double MAX_steps = 360000; // 36 cm, 100 steps = 1 mm
>>>>>>> 5fd11337237be020aebc150ca3951e329d1dabdf
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
bool mocon = false; // chenge true when motor connected
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
  // posible additional features...
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH Shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

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
  if(!isZero){
    if(mocon){
      toZero();
    }
    isZero = true;
  }
  // wait for a new client:
  checkClient(printData());
    // if command readded true set command
  if(CR == true){setCommand(command);CR = false;}
    // play condition
  if(play){
    // ***Stepper commands***
    steps = 100 * I_mm * direction * i;
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
          Serial.print("Interval: ");
          Serial.print(i - 1);
          Serial.print(", Run to: ");
          Serial.print(offset + steps);
          Serial.print(", time: ");
          Serial.println(start_time);
        }
        else{
          play = false;
        }
        
      }
    }
    else{
      if(direction == -1){
        if((millis()/1000) - start_time >= I_s){
          if(steps > 0 && i <= I_count){
            i++;
            start_time = millis()/1000;
            if(mocon){
              stepper.moveTo(offset - steps);
              stepper.runToPosition();
            }
          Serial.print("Interval: ");
          Serial.print(i);
          Serial.print("Run to: ");
          Serial.print(offset - steps);
          Serial.print(", time: ");
          Serial.println(start_time);
          }
          else{
            play = false;
          }
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
    Serial.print("Play position: ");
    Serial.print(offset);
    Serial.print(" direction: ");
    Serial.print(direction);
    play = true;
    start_time = millis()/1000;
  }else if(c == "reset"){
    Serial.println("Reset");
    play = false;
    I_mm = 0; offset = 0; I_s = 0; I_count = 0; direction = 0; ready = 0; steps = 0;
    Serial.println(I_mm + offset + I_s + I_count + direction + ready + steps);
    if(mocon){
      backZero();
    }
  }else{
    // parse values
    Serial.println(c);
    String* p = split(c, '|', qtde);
    I_count = p[0].toDouble();
    I_mm = p[1].toDouble();
    I_s = p[2].toDouble();
    direction = p[3].toDouble();
    offset = p[4].toDouble();
    delete[] p;
    Serial.print("Intervals: ");
    Serial.println(I_count);
    Serial.print("Interval mm: ");
    Serial.println(I_mm);
    Serial.print("Interval s: ");
    Serial.println(I_s);
    Serial.print("Dir: ");
    Serial.println(direction);
    Serial.print("Offset: ");
    Serial.println(offset);
    if(mocon){
      stepper.moveTo(offset * 100);
      stepper.runToPosition();
    }
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
  }
  else{
    Serial.println("Go to zero");
    // Set direction counterclockwise
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    digitalWrite(dirPin, LOW);
    while(digitalRead(limPin) > 0){
<<<<<<< HEAD
      checkClient("Running to reference point");
=======
>>>>>>> 5fd11337237be020aebc150ca3951e329d1dabdf
      digitalWrite(dirPin, HIGH);
      delay(1);      
      digitalWrite(dirPin, LOW);
      delay(1);
    }
    Serial.println();
    Serial.println("In zere");
    stepper.setCurrentPosition(0);
  }
}
void backZero(){
  stepper.moveTo(0);
  stepper.runToPosition();
}
void checkClient(String m){
  EthernetClient client = server.available();

  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer:
      client.flush();
      alreadyConnected = true;
    }
    //Serial.println();
    if(client.available() > 0) {
      // read the string incoming from the client:
        if(m != "Running to reference point"){
          command = client.readStringUntil('*');
        }
        client.println(m);
        CR = true; // command readed true
    }
  }
  client.stop();
}
// Optional spin method for motor (just incase)
// spinMotor(int _steps, int _direction){
//   digitalWrite(dirPin, _direction);
//   while(s > 0 || stepsTaken < MAX_s){
//     digitalWrite(dirPin, HIGH);
//     delay(1);      
//     digitalWrite(dirPin, LOW);
//     delay(1);
//     s--;
//     stepsTaken++;
//   }
// }
