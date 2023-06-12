/*
 Chat Server

 A simple server that distributes any incoming messages to all
 connected clients.  To use, telnet to your device's IP address and type.
 You can see the client's input in the serial monitor as well.
 Using an Arduino WIZnet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */
#include <AccelStepper.h>
#include <SPI.h>
#include <Ethernet.h>
#define dirPin 2
#define stepPin 3
#define motorinterfaceType 1
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
String command;
char c;
int qtde;
// telnet defaults to port 23
EthernetServer server(23);
bool alreadyConnected = false; // whether or not the client was connected previously
double I_mm = 0;
double offset = 0;
double I_s = 0;
double direction = 0;
double I_count = 0;
bool play = false;
bool CR = false; // command readed
double steps = 0;
double i = 1;
double start_time = 0;
bool isZero = false;
void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH Shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

  // initialize the Ethernet device
  Ethernet.begin(mac, ip, myDns, gateway, subnet);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  if(Serial){
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }

  }

  // start listening for clients
  server.begin();
  if(Serial){
    Serial.print("Server address:");
    Serial.println(Ethernet.localIP());
  }

}

void loop() {
  if(!isZero){
    toZero();
    isZero = true;
  }
  // wait for a new client:
  EthernetClient client = server.available();

  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer:
      // Send info to client!!!
      client.flush();
      if(Serial){Serial.println("A new client");}
      
      client.println(printData());
      alreadyConnected = true;
    }
    //Serial.println();
    if(client.available() > 0) {
      // read the bytes incoming from the client:
      if(command != client.readStringUntil('*')){
        // if message is chage update it;
        command = client.readStringUntil('*');
        CR = true; // command readed true
      }

    }
  }
  if(CR){
    // if command readded true set command
    setCommand();
    CR = false;
  }
  if(play){
    // play true
    // ***Stepper commands***
    steps = 100 * I_mm * direction * i;
    // test timer logic..
    if((millis()/1000) - start_time >= I_s){
      if(steps < MAX_steps && i <= I_count){
        if(Serial){Serial.println(steps);}
        
        i++;
        start_time = millis()/1000;
        // moveToPosition(steps)
        // runToPosition
        // Still need to figure out how to do interval timer...
        // There is many possible option to use..
      }
      else{
        // moveToPosition(MAX_steps)
        // runToPosition
        // delay(1000)
        // moveToPosition()
        // runToPosition
        play = false;
      }
    }
  }
}
// ******LOOP END*******
String printData(){
  // return info to user
  double ready = 0;
  if(steps > 0){
    ready = steps / MAX_steps * 100;
  }
  String state = "Connected to: ";
  state += String(Ethernet.localIP());
  state += "\n__Values__\n";
  state += "Interval length mm:";
  state += String(I_mm);
  state += "\nInterval offset: ";
  state += String(offset);
  state += "\nInterval time: ";
  state += String(I_s);
  state += " s\nIntervals: ";
  state += String(I_count);
  state += "\nDirection: ";
  state += String(direction);
  state += "\nPlay: ";
  state += String(play);
  state += "\nReady: ";
  state += String(ready);
  state += "%";
  return state;
}
void setCommand(){
  if(command == ""){

  }else if(command == "min"){
      // ***Stepper commands***
      // moveToPosition(0);
      // runToPosition();
  }else if(command == "max"){
      // ***Stepper commands***
      // moveToPosition(MAX_steps);
      // runToPosition();
  }else if(command == "play"){
      play = true;
      start_time = millis()/1000;
  }else if(command == "pause"){
      // pause condition
      play = false;
  }else if(command == "reset"){
      play = false;
      I_mm = 0;
      I_s = 0;
      I_count = 0;
      direction = 0;
      // reset condition
      // ***Stepper commands***
      // moveToPosition(0);
      // runToPosition();
  }else{
      // parse values
      String* p = split(command, '|', qtde);
      I_mm = p[0].toDouble();
      I_s = p[1].toDouble();
      I_count = p[2].toDouble();
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
  // ***Stepper commands***
  // moveToPosition(-300);
  // runToPosition();
  // limit switch break..
  // currentPosition(0);
}
