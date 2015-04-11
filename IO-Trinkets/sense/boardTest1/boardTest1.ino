#include <Wire.h>

#define SELECTOR_PIN 4
#define ENABLE_PIN 3

#define SLAVE_ADDRESS 0x03

boolean senseLight;
void setup() 
{
  senseLight = true;
  pinMode(SELECTOR_PIN, OUTPUT); 
  pinMode(ENABLE_PIN, OUTPUT);
  
  digitalWrite(ENABLE_PIN, HIGH);
  digitalWrite(SELECTOR_PIN, HIGH);
  
  Wire.begin(SLAVE_ADDRESS);
  //Wire.onReceive(receiveData);
  //Wire.onRequest(sendData);
}


void loop() {
  //digitalWrite(SELECTOR_PIN, senseLight);
  //senseLight = !senseLight;    
}
