/*
  - PROTOCOL - 
  All Responses first byte is OPCODE that it is responding to
  First byte received is the opcode:
    0x00: STATUS
      OPERAND: 
        none
      RESPONSE: 
        STATUS_READY 0x01
        STATUS_FAULT 0xFF
    0x01: STREAM LIGHT
      OPERAND:
        none
    0x02: STREAM SOUND
      OPERAND:
        none
    ------
    may add stream buffered sound/light... but unlikely
    ------
    0xFE: STOP_STREAMING
      OPERAND:
        none
      RESPONSE:
        none
*/    
#include <Wire.h>

#define SELECTOR_PIN 4
#define ENABLE_PIN 3

#define SLAVE_ADDRESS 0x03

#define BMASK 0x3F
#define DMASK 0x60

#define OPCODE_STATUS 0x00
#define OPCODE_STREAM_LIGHT 0x01
#define OPCODE_STREAM_SOUND 0x02
#define OPCODE_STOP_STREAM 0xFE

enum SENSE_MODE {OFF = 0x00, LIGHT = 0x01, SOUND = 0x02};
volatile boolean StatusReq = false;
volatile SENSE_MODE sense;
void setup() 
{
  //Serial.begin(9600);
  //Serial.println("Starting");
  sense = OFF;
  pinMode(SELECTOR_PIN, OUTPUT); 
  pinMode(ENABLE_PIN, OUTPUT);
  
  // set Port B for input
  //DDRB = B00000000; 
  // set Port D for input
  //DDRD = B00000000;

  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(SELECTOR_PIN, LOW);
  
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}


void loop() {
  /*byte data = (PINB & BMASK) | ((PIND & DMASK) << 1);
  if(sense == LIGHT)  
    Serial.println(data);*/
  //digitalWrite(SELECTOR_PIN, senseLight);
  //senseLight = !senseLight;    
}

// callback for received data
void receiveData(int byteCount)
{
  while(Wire.available())
  {
    switch(Wire.read())
    {
      case OPCODE_STATUS:
        StatusReq = true;
        break;
      case OPCODE_STREAM_LIGHT:
        sense = LIGHT;
        digitalWrite(SELECTOR_PIN, HIGH);        
        digitalWrite(ENABLE_PIN, HIGH);
        break;
      case OPCODE_STREAM_SOUND:
        sense = SOUND;
        digitalWrite(SELECTOR_PIN, LOW);        
        digitalWrite(ENABLE_PIN, HIGH);
        break;
      case OPCODE_STOP_STREAM:
        sense = OFF;
        digitalWrite(SELECTOR_PIN, LOW);        
        digitalWrite(ENABLE_PIN, LOW);
        break;      
    }
  }
}
	 
// callback for sending data
void sendData()
{
  if(StatusReq) {
    byte resp[2] = {OPCODE_STATUS, sense};
    Wire.write(resp, 2);
    StatusReq = false;
  }
  else
  {
    byte data = (PINB & BMASK) | ((PIND & DMASK) >> 5);
    Wire.write(&data, 1);
  }
}
