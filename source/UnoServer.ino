/*
Arduino telnet server stepper motor control
created 2023 Teemu Sekki

 */
#include <ezButton.h>
#include <AccelStepper.h>
#include <SPI.h>
#include <Ethernet.h>

#define dirPin 2
#define stepPin 3
#define motorinterfaceType 1
#define limPin 7

ezButton limitSwitch(limPin);
AccelStepper stepper = AccelStepper(motorinterfaceType, stepPin, dirPin);
const double MAX_steps = 360000; // 36 cm, 100 steps = 1 mm
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
String cStr = "";
String command = "";
char c;
int qtde;
// telnet defaults to port 23
EthernetServer server(23);
double ready = 0;
bool alreadyConnected = false; // whether or not the client was connected previously
// parameters start ****
double I_mm = 0;
double offset = 0;
double I_s = 0;
double direction = 0;
double I_count = 0;
// parameters end *******
bool play = false; // play state
bool CR = false; // command readed
double steps = 0; // measure taken steps
double i = 1;
double start_time = 0;
bool isZero = false; // check the reference point
int limState; 
void setup() {
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(30);
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
    toZero();
    Serial.println("In reference point");
    isZero = true;
  }
  // wait for a new client:
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
        command = client.readStringUntil('*');
        client.println(printData());
        CR = true; // command readed true
    }
  }
    // if command readded true set command
  if(CR == true){setCommand(command);CR = false;}
    // play condition
  if(play){
    // ***Stepper commands***
    steps = 100 * I_mm * direction * i;
    // test timer logic..
    if((millis()/1000) - start_time >= I_s){
      if(steps < MAX_steps && i <= I_count){
        i++;
        start_time = millis()/1000;
        stepper.moveToPosition(steps)
        stepper.runToPosition()
      }
      else{
        // check if needed ************
        // If last step is less than interval lenght...
        // stepper.moveToPosition(MAX_steps)
        // stepper.runToPosition()
        // delay(1000 * I_s)
        play = false;
        iZero = false;
      }
      
    }
  }
  client.stop();
}
// ******LOOP END*******
String printData(){
  // return info to user
  // add reference point info
  if(steps > 0){
    ready = steps / MAX_steps * 100;
  }
  String state = "Connected to: 192.168.1.177";
  state += "\n__Values__\n";
  state += "Interval length mm:";
  state += String(I_mm);
  state += "\nInterval offset:";
  state += String(offset);
  state += "\nInterval time:";
  state += String(I_s);
  state += " s\nIntervals:";
  state += String(I_count);
  state += "\nDirection:";
  state += String(direction);
  state += "\nPlay:";
  state += String(play);
  state += "\nReference:";
  state += String(isZero);
  state += "\nReady:";
  state += String(ready);
  state += "%";
  return state;
}
void setCommand(String c){

  if(c == "info"){
    // returns only info
  }else if(c == "min"){
    // ***Stepper commands***
    stepper.moveToPosition(0);
    stepper.runToPosition();
  }else if(c == "max"){
    // ***Stepper commands***
    stepper.moveToPosition(MAX_steps);
    stepper.runToPosition();
  }else if(c == "move"){
    stepper.moveToPosition(offset);
    stepper.runToPosition();
  }else if(c == "play"){
    play = true;
    start_time = millis()/1000;
  }else if(c == "pause"){
    // pause condition
    play = false;
  }else if(c == "reset"){
    play = false;
    I_mm = 0;
    ofsett = = 0;
    I_s = 0;
    I_count = 0;
    direction = 0;
    ready = 0;
    toZero();
  }else{
    // parse values
    String* p = split(c, '|', qtde);
    I_count = p[0].toDouble();
    I_mm = p[1].toDouble();
    I_s = p[2].toDouble();
    direction = p[3].toDouble();
    offset = p[4].toDouble();
    delete[] p;
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
    digitalWrite(dirPin, LOW);
    while(digitalRead(limPin) > 0){
      digitalWrite(dirPin, HIGH);
      delay(1);
      digitalWrite(dirPin, LOW);
      delay(1);
    }
    Serial.println();
    stepper.setCurrentPosition(0);
  }
}
